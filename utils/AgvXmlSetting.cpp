/*
 * AgvXmlSettingHelper.cpp
 *
 *  Created on: 2016-10-25
 *      Author: zxq
 */

#include <stdio.h>
#include <stdlib.h>
#include "AgvXmlSetting.h"
#include "AgvXml.h"

CAgvXmlSetting::CAgvXmlSetting()
{
}

CAgvXmlSetting::~CAgvXmlSetting()
{
    Close();
}


int CAgvXmlSetting::LoadContents(const char *fname)
{
    content.clear();
    xmlFileName = fname;
    return CAgvXml::ReadCfgDataFromXml(fname, &content);
}


string CAgvXmlSetting::GetSectionValue(const string &section, const char *def)
{
    map<string, string>::iterator it;
    it = content.find(section);
    if (it == content.end())
    {
        SaveSectionValue(section, def);
        return string(def);
    }
    return it->second;
}

int CAgvXmlSetting::GetSectionValue(const string& section, const int &def)
{
    char buf[20] = {0};
    sprintf(buf, "%d", def);
    string ret = GetSectionValue(section, buf);
    return atoi(ret.c_str());
}

bool CAgvXmlSetting::GetSectionValue(const string& section, const bool &def)
{
    char buf[20] = {0};
    sprintf(buf, "%d", def);
    string ret = GetSectionValue(section, buf);
    return atoi(ret.c_str());
}

double CAgvXmlSetting::GetSectionValue(const string& section, const double &def)
{

    char buf[20] = {0};
    sprintf(buf, "%f", def);
    string ret = GetSectionValue(section, buf);
    return atof(ret.c_str());
}


int CAgvXmlSetting::SaveSectionValue(const string &section, const char *value)
{
    content.erase(section);
    content.insert(pair<string, string>(section, value));
    return CAgvXml::WriteCfgDataToXml(xmlFileName.c_str(), section.c_str(), value);
}

int CAgvXmlSetting::SaveSectionValue(const string& section, const int &value)
{
    char buf[20] = {0};
    sprintf(buf, "%d", value);
    return SaveSectionValue(section, buf);
}

int CAgvXmlSetting::SaveSectionValue(const string& section, const bool &value)
{
    char buf[20] = {0};
    sprintf(buf, "%d", value);
    return SaveSectionValue(section, buf);
}

int CAgvXmlSetting::SaveSectionValue(const string& section, const double &value)
{
    char buf[20] = {0};
    sprintf(buf, "%f", value);
    return SaveSectionValue(section, buf);
}

int CAgvXmlSetting::Close()
{
    content.clear();
    return 0;
}
