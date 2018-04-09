#ifndef AGVINI_H
#define AGVINI_H

#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpointer.h>
#include <map>
#include <string.h>

using namespace std;

class CAgvXml
{
public:
    CAgvXml();
    ~CAgvXml();

public:
    static int WriteCfgDataToXml(const char *iniFile, const char *entry, const char *val);
    static int ReadCfgDataFromXml(const char *iniFile, map<string,string> *cfgData);
    static string getMapString(map<string,string> &m, string key);

};

#endif // AGVINI_H
