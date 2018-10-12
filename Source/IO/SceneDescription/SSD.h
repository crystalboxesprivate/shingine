#pragma once
namespace SSD
{
    const unsigned char NodeBegin = 0xaa;
    const unsigned char NodeEnd = 0xab;
    const unsigned char AttributeBegin = 0xba;
    const unsigned char AttributeEnd = 0xbb;

    struct SHeader
    {
        char Signature[3];
        unsigned char Version;
    };
    struct SAttribute;

    struct SNode
    {
        unsigned short ID;
        unsigned short ParentID;
        unsigned char Type;
        unsigned char NameLength;
        char* Name;
        unsigned char AttributeCount;
        unsigned char NodeCount;
        SAttribute** Attributes;
        SNode** Nodes;
    };

    struct SAttribute
    {
        unsigned char NameLength;
        char* Name;
        unsigned char DataType;
        unsigned int ElementCount;
        unsigned char* Values;
    };

};
