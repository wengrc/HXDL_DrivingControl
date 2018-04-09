/*
 * AgvXmlSettingHelper.h
 *
 *  Created on: 2016-10-25
 *      Author: zxq
 */

#ifndef AGVXMLSETTINGHELPER_H_
#define AGVXMLSETTINGHELPER_H_

#include <string>
#include <map>
#include <iostream>

using namespace std;


class CAgvXmlSetting
{
    public:
        CAgvXmlSetting();
        virtual ~CAgvXmlSetting();

        int LoadContents(const char *fname);

        inline const string &FileName() {return xmlFileName;}

        string GetSectionValue(const string &section, const char *def);

        int GetSectionValue(const string &section, const int &def);

        bool GetSectionValue(const string &section, const bool &def);

        double GetSectionValue(const string &section, const double &def);


        int SaveSectionValue(const string &section, const char *value);

        int SaveSectionValue(const string &section, const int &value);

        int SaveSectionValue(const string &section, const bool &value);

        int SaveSectionValue(const string &section, const double &value);

        int Close();

    private:
        string                  xmlFileName;
        map<string, string>     content;

};

#endif /* AGVXMLSETTINGHELPER_H_ */
