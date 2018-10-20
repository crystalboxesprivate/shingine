# shingine scene exporter for houdini
# *.ssd (shingine scene description)
# format structure
'''
HEADER 
{
    signature U24
    version U8
}
NODE
{
    node_begin U8
    unique_id U32
    name_length U8
    name U8[name_length]
    attribute_count U8
    node_count U8
    attributes ATTRIBUTE[attribute_count]
    nodes NODE[node_count]
    node_end U8
}
ATTRIBUTE
{
    attr_begin U8
    name_length U8
    name U8[name_length]
    data_type_char_length U8
    data_type U8[data_type_char_length]
    byte_count U32
    element_count U32
    value U8[element_count]
    attr_end U8
}
FILE
{
    header HEADER
    node_count U16
    nodes NODE[node_count]
}
'''

from enum import IntEnum
import struct

class DataType(IntEnum):
    NONE = 0
    BYTE = 1
    UINT = 2
    FLOAT = 3
    INT = 4
    INT16 = 5
    UINT16 = 6
    CHAR = 7
    UID = 8
    SERIALIZED_CLASS = 9

class LightType(IntEnum):
    POINT = 0

class NodeType(IntEnum):
    OBJECT = 0
    DATA = 1

class DrawType(IntEnum):
    WIRE_FRAME = 0
    FILL = 1
    POINTS = 2

data_type_name_map = {
    DataType.BYTE : "unsigned char",
    DataType.INT : "int",
    DataType.INT16 : "short",
    DataType.UINT : "unsigned int",
    DataType.UINT16 : "unsigned short",
    DataType.FLOAT : "float",
    DataType.CHAR : "char",
    DataType.UID : "uid",
    DataType.SERIALIZED_CLASS : "SerializedClass"
}

NextID = 50

class Node:
    def __init__(self, name="Node", type=NodeType.OBJECT):
        # zero is the scene root
        global NextID
        self.name = name
        self.unique_id = NextID
        self.attributes = []
        self.nodes = []
        self.type = type
        NextID += 1

class Attribute:
    def __init__(self, name, data_type, value, single_element=False):
        self.name = name
        self.data_type = data_type
        self.single_element = single_element
        self.value = value

#node_name_to_id = {}
node_id_to_parent_id = {}
node_name_to_ssd_node = {}
hom_nodes = []
nodes = []
meshes = {}
materials = {}

# get all hom nodes
for id, node in enumerate(hou.node("/obj").children()):
    if node.type().name() not in ["light", "null", "geo"]:
        continue
    #node_name_to_id[node.name()] = id + 1
    new_node = Node("Object", type=NodeType.OBJECT)
    new_node.attributes.append(Attribute("Id", DataType.UINT, id + 1, True))
    # add object metadata
    metadata_node = Node("ObjectMetadata")
    metadata_node.attributes.append(Attribute("Name", DataType.CHAR, node.name(), True))
    metadata_node.attributes.append(Attribute("Tag", DataType.CHAR, "default", True))
    metadata_node.attributes.append(Attribute("Layer", DataType.CHAR, "default", True))
    new_node.nodes.append(metadata_node)
    nodes.append(new_node)
    node_name_to_ssd_node[node.name()] = new_node

    hom_nodes.append(node)

# set parent ids
for id, hom_node in enumerate(hom_nodes):
    parent_id = 0
    if hom_node.inputs():
        input_node = hom_node.inputs()[0]
        if input_node.name() not in node_name_to_ssd_node.keys():
            continue
        parent_id = node_name_to_ssd_node[input_node.name()].unique_id
    node_id_to_parent_id[id] = parent_id

# read node attributes
for id, node in enumerate(hom_nodes):
    # collect the local transform matrix, write to the attribute
    local_transform = node.localTransform()
    position = local_transform.extractTranslates()
    quat = hou.Quaternion(local_transform.extractRotationMatrix3())
    scale = local_transform.extractScales()

    transform_node = Node("Transform")
    transform_node.attributes.append(Attribute("ParentID", DataType.UID, node_id_to_parent_id[id], True))
    transform_node.attributes.append(Attribute("IsDynamic", DataType.BYTE, 0, True))
    transform_node.attributes.append(Attribute("LocalPosition", DataType.FLOAT, position))
    transform_node.attributes.append(Attribute("LocalRotation", DataType.FLOAT, quat))
    transform_node.attributes.append(Attribute("LocalScale", DataType.FLOAT, scale))
    nodes[id].nodes.append(transform_node)
    # find lights
    if node.type().name() == "hlight::2.0":
        light_node = Node("Light")
        light_node.attributes.append(Attribute("Color", DataType.FLOAT, node.parmTuple("light_color").eval()))
        light_node.attributes.append(Attribute("Exposure", DataType.FLOAT, node.parm("light_exposure").eval(), True))
        light_node.attributes.append(Attribute("Intensity", DataType.FLOAT, node.parm("light_intensity").eval(), True))
        nodes[id].nodes.append(light_node)
    
    # find geo
    if node.type().name() == "geo":
        # the geo can contain non-tris
        # triangulate it first and calculate point normals after
        display = node.displayNode()
        divide = node.createNode("divide")
        normal = node.createNode("normal")
        normal.parm("type").set(0)
        divide.setInput(0, display)
        normal.setInput(0, divide)
        normal.setDisplayFlag(1)
        normal.setRenderFlag(1)
        # read geometry data from triangulated node
        geometry = normal.geometry()
        positions = []
        normals = []
        indices = []
        texcoord = []
        for point in geometry.points():
            positions.append(point.position().x())
            positions.append(point.position().y())
            positions.append(point.position().z())

            normal_value = point.attribValue("N")
            normals.append(normal_value[0])
            normals.append(normal_value[1])
            normals.append(normal_value[2])

            # deal with uvs later
            texcoord.append(0.0)
            texcoord.append(0.0)
            texcoord.append(0.0)

        for face in geometry.prims():
            for vert in face.vertices():
                indices.append(vert.point().number())
        
        # add attributes
        geometry_node = Node("Mesh")
        geometry_node.attributes.append(Attribute("Name", DataType.CHAR, node.name(), True))
        geometry_node.attributes.append(Attribute("Indices", DataType.UINT, indices))
        geometry_node.attributes.append(Attribute("Normals", DataType.FLOAT, normals))
        geometry_node.attributes.append(Attribute("Positions", DataType.FLOAT, positions))
        geometry_node.attributes.append(Attribute("TexCoord", DataType.FLOAT, texcoord))
        meshes[node.name()] = geometry_node

        # delete temp houdini nodes
        normal.destroy()
        divide.destroy()

        renderer_node = Node("Renderer")
        renderer_node.attributes.append(Attribute("DrawType", DataType.BYTE, DrawType.FILL, True))
        renderer_node.attributes.append(Attribute("Enabled", DataType.BYTE, 1, True))
        renderer_node.attributes.append(Attribute("MeshReference", DataType.UID, geometry_node.unique_id, True))

        material_node = None
        # find material
        material_id = 0
        if node.parm("shop_materialpath").eval():
            material_hom_node = hou.node(node.parm("shop_materialpath").eval())
            if material_hom_node.type().name() == "principledshader::2.0":
                material_name = material_hom_node.name()
                if material_name not in materials.keys():
                    material_node = Node("Material")
                    material_node.attributes.append(Attribute("Name", DataType.CHAR, material_name, True))
                    material_node.attributes.append(Attribute("ShaderName", DataType.CHAR, "default", True))
                    material_node.attributes.append(Attribute("DiffuseColor", DataType.FLOAT, node.parmTuple("basecolor").eval()))
                    materials[material_name] = material_node
                    material_id = material_node.unique_id
        renderer_node.attributes.append(Attribute("MaterialReference", DataType.UID, material_id, True))

        nodes[id].nodes.append(renderer_node)
# add meshes 
nodes.extend(meshes.values())
nodes.extend(materials.values())

# add shaders
shader_node = Node("Shader")
shader_node.attributes.append(Attribute("Language", DataType.CHAR, "GLSL", True))
shader_source_vertex = Node("ShaderSource")
shader_source_vertex.attributes.append(Attribute("Type", DataType.UINT16, 1, True))
shader_source_vertex.attributes.append(Attribute("Source", DataType.CHAR, "int main() \n { \n return 0; \n }", True))
shader_source_fragment = Node("ShaderSource")
shader_source_fragment.attributes.append(Attribute("Type", DataType.UINT16, 1, True))
shader_source_fragment.attributes.append(Attribute("Source", DataType.CHAR, "int mainMain() \n { \n int x = 2; \n return 0; \n }", True))
shader_node.attributes.append(Attribute("Source", DataType.SERIALIZED_CLASS, [shader_source_vertex, shader_source_fragment]))
nodes.append(shader_node)

version = 1
signature = "SSD"
node_begin = 0xaa
node_end = 0xab
attr_begin = 0xba
attr_end = 0xbb
character_limit = 32

def get_uint16_to_bytes(val):
    val = val & 0xffff
    return ((val >> 8) & 0xff, val & 0xff)

def get_uint32_to_bytes(val):
    val = val & 0xffffffff
    return (
        (val >> 24) & 0xff, 
        (val >> 16) & 0xff, 
        (val >> 8) & 0xff, 
        val & 0xff)

def string_to_bytes(s):
    ba = bytearray(s)
    ba.append(0)
    return ba
    
def attribute_to_bytes(attr):
    attr_data = bytearray()
    attr_data.append(attr_begin)
    attr_name = string_to_bytes(attr.name[:255])
    attr_data.append(len(attr_name) & 0xff)
    attr_data.extend(attr_name)

    # data type name
    data_type_name = string_to_bytes(data_type_name_map[attr.data_type])
    attr_data.append(len(data_type_name) & 0xff)
    attr_data.extend(data_type_name)

    # handle strings
    if attr.data_type == DataType.CHAR:
        if attr.single_element:
            # byte count
            attr_data.extend(get_uint32_to_bytes(len(attr.value) + 1))
            # element count
            attr_data.extend(get_uint32_to_bytes(1))
            # value
            attr_data.extend(attr.value)
            attr_data.append('\0')
        else:
            element_count = len(attr.value)
            values = bytearray()
            for x in range(element_count):
                values.extend(attr.value[x])
                values.append('\0')
            # byte count
            attr_data.extend(get_uint32_to_bytes(len(values)))
            # element count
            attr_data.extend(get_uint32_to_bytes(element_count))
            # value
            attr_data.extend(values)
    elif attr.data_type == DataType.SERIALIZED_CLASS:
        element_count = len(attr.value)
        # byte count
        attr_data.extend(get_uint32_to_bytes(element_count))
        # element count
        attr_data.extend(get_uint32_to_bytes(element_count))
        # value
        for c in attr.value:
            attr_data.extend(node_to_bytes(c))
    else:
        values = attr.value
        if attr.single_element:
            element_count = 1
            values = [attr.value,]
        element_count = len(values)
        byte_count = element_count
        unpacked_values = bytearray()
        for x in range(len(values)):
            if (attr.data_type == DataType.FLOAT):
                byte_count = element_count * 4
                unpacked_values.extend(bytearray(struct.pack("f", values[x])))
            if (attr.data_type == DataType.UINT or attr.data_type == DataType.UID \
                or attr.data_type == DataType.INT):
                byte_count = element_count * 4
                unpacked_values.extend(get_uint32_to_bytes(values[x]))
            if (attr.data_type == DataType.UINT16 or attr.data_type == DataType.INT16):
                byte_count = element_count * 2
                unpacked_values.extend(get_uint16_to_bytes(values[x]))
            if (attr.data_type == DataType.BYTE):
                unpacked_values.append(values[x] & 0xff)
        # byte count
        attr_data.extend(get_uint32_to_bytes(byte_count))
        # element count
        attr_data.extend(get_uint32_to_bytes(element_count))
        # value
        attr_data.extend(unpacked_values)

    attr_data.append(attr_end)
    return attr_data

def node_to_bytes(node):
    node_data = bytearray()
    node_data.append(node_begin)
    node_data.extend(get_uint32_to_bytes(node.unique_id))
    # node_data.append(node.type & 0xff)

    node_name = string_to_bytes(node.name[:255])
    node_data.append(len(node_name) & 0xff)
    node_data.extend(node_name)

    node_data.append(len(node.attributes) & 0xff)
    node_data.append(len(node.nodes) & 0xff)

    for x in range(len(node.attributes)):
        node_data.extend(attribute_to_bytes(node.attributes[x]))
    for x in range(len(node.nodes)):
        node_data.extend(node_to_bytes(node.nodes[x]))
    node_data.append(node_end)
    return node_data

# file select
out_file = hou.expandString(hou.ui.selectFile(pattern="*.ssd"))

if not out_file.endswith(".ssd"):
    out_file += ".ssd"

data = bytearray(signature)
data.append(version)
data.extend(get_uint16_to_bytes(len(nodes)))

for node in nodes:
    # write nodes
    data.extend(node_to_bytes(node))
    
f = open(out_file, "w+b")
f.write(data)
f.close()
