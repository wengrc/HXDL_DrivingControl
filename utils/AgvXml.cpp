#include "AgvXml.h"



CAgvXml::CAgvXml()
{
}


CAgvXml::~CAgvXml()
{
}





int CAgvXml::ReadCfgDataFromXml(const char *iniFile, map<string, string> *cfgData)
{
   if(iniFile == NULL || cfgData == NULL)
   {
      perror("input param is NULL:");
      return -1;
   }

    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcur = NULL;


    xmlKeepBlanksDefault(0);
    pdoc = xmlReadFile (iniFile, "UTF-8", XML_PARSE_RECOVER);

    if(pdoc == NULL)
    {
        perror("open xmlDoc error:");
        return -1;
    }


    pcur = xmlDocGetRootElement (pdoc);

    if(pcur == NULL)
    {
        perror("Get rootElement error:");
        return -1;
    }

    pcur = pcur->children;
    while (pcur != NULL)
    {
        xmlChar* key;
        key = xmlNodeListGetString(pdoc, pcur->xmlChildrenNode, 1);
        string snodeName((const char*)pcur->name);
        string snodeVal((const char*)key);
//        printf("<%s = %s>\n",pcur->name, key);
        cfgData->insert(pair<string, string>(snodeName, snodeVal));
        xmlFree(key);
        pcur = pcur->next;
    }


    xmlFreeDoc (pdoc);
    xmlCleanupParser();
    return 0;
}




int CAgvXml::WriteCfgDataToXml(const char *iniFile, const char *entry, const char *val)
{
  int cnt = 0;
  if(iniFile == NULL)
  {
      perror("cfg file is NULL:");
      return -1;
  }

  xmlDocPtr pdoc = NULL;
  xmlNodePtr prootNode = NULL;

  xmlKeepBlanksDefault(0);
  xmlIndentTreeOutput = 1 ;

  if(-1 == access(iniFile,F_OK))  //FILE IS NOT EXIST
  {
     pdoc = xmlNewDoc(BAD_CAST "1.0");
     prootNode = xmlNewNode(NULL, BAD_CAST "cfg");
     xmlDocSetRootElement(pdoc, prootNode);
     xmlNodePtr pnodePtr = xmlNewChild(prootNode, NULL, BAD_CAST(entry),BAD_CAST(val));
     if(pnodePtr == NULL)
     {
        perror("add node error:");
        return -1;
     }
     xmlSaveFormatFile(iniFile, pdoc, 1);
     xmlFree(pdoc);
     return 0;
  }

  pdoc = xmlReadFile (iniFile, "UTF-8", XML_PARSE_RECOVER);

  if (pdoc == NULL)
  {
      printf ("cfg ini file open error:");
      return -1;
  }


  prootNode = xmlDocGetRootElement (pdoc);

  if (prootNode == NULL)
  {
      printf("Get root node error:");
      return -1;
  }
  xmlNodePtr pcur = prootNode;
  pcur = pcur->children;

  while (pcur != NULL)
  {
      if(!xmlStrcmp(pcur->name, BAD_CAST(entry)))
      {
         cnt++;
         xmlNodeSetContent(pcur, BAD_CAST(val));
      }
      pcur = pcur->next;
  }

  if(cnt == 0)
  {
      xmlNodePtr pnodePtr = xmlNewChild(prootNode, NULL, BAD_CAST(entry),BAD_CAST(val));
      if(pnodePtr == NULL)
      {
         perror("add node error:");
         return -1;
      }

  }
  xmlSaveFormatFile(iniFile, pdoc, 1);
  xmlFree(pdoc);
  return 0;
}


string CAgvXml::getMapString(map<string, string> &m, string key)
{
    if(m.count(key) > 0)
    {
        return m[key];
    }
    return NULL;
}



