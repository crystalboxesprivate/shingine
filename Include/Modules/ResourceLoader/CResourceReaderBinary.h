#pragma once
#include "IResourceReader.h"

namespace SSD { struct SHeader; struct SNode; struct SAttribute; }

class CResourceReaderBinary : public IResourceReader
{
public:
    CResourceReaderBinary(const CString &fileName);
    virtual ~CResourceReaderBinary();
    virtual bool Open();
    virtual void Close();
    virtual void ReadNodes(std::vector<IDataNode*> &nodes);
    virtual CString GetLastError() { return LastError; }
    // virtual 
private:
    void ReadHeader(SSD::SHeader &header);
    void ReadUShort(unsigned short &val);
    void ReadByte(unsigned char &val);
    void ReadUInt32(unsigned int &val);
    SSD::SNode* ReadNode();
    SSD::SAttribute* ReadAttribute();

    CString FileName;
    CString LastError;
    std::ifstream FileStream;
};