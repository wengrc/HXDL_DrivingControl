#ifndef AGVJSON_H
#define AGVJSON_H

#include <json/json.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
using namespace std;

typedef struct json_data_out
{
    string pid;
    string disable;
    string es0_ver;
    string es1_ver;
    string es2_ver;
    string es3_ver;
    string es4_ver;
    string url;
    string md5;

}JsonDataOut;

class CAgvJson
{
public:
    CAgvJson();
    ~CAgvJson();

public:
    const char *EncodeStringToServer(char *pid,char *es0_ver,char *es1_ver,char *es2_ver,char *es3_ver,char *es4_ver,char *dis);
    int DecodeStringFromServer(const char *json_data_in,JsonDataOut *json_data_out);

private:
    int json_loop_object(json_object *obj,JsonDataOut *json_data_out);
    int json_get_value(json_object *obj,char *key,JsonDataOut *json_data_out);
    json_object *json;

};

#endif // AGVJSON_H
