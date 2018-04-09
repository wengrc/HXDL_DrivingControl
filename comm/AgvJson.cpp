#include "AgvJson.h"

CAgvJson::CAgvJson()
{
    json = NULL;
}

CAgvJson::~CAgvJson()
{
    json_object_put(json);
}

int CAgvJson::json_loop_object(json_object *obj,JsonDataOut *json_data_out)
{
    if(NULL == obj)
    {
        return -1;
    }

    json_object_object_foreach(obj,key,val)
    {
        json_get_value(val,key,json_data_out);
    }

    return 0;
}

int CAgvJson::json_get_value(json_object *obj,char *key,JsonDataOut *json_data_out)
{
    if(NULL == obj)
    {
        return -1;
    }

    json_type type=json_object_get_type(obj);

    if(type == json_type_string)
    {
        if(0 == strcmp(key,"pid"))
        {
            json_data_out->pid = json_object_get_string(obj);
            return 0;
        }

        if(0 == strcmp(key,"es0"))
        {
            json_data_out->es0_ver = json_object_get_string(obj);
            return 0;
        }

        if(0 == strcmp(key,"es1"))
        {
            json_data_out->es1_ver = json_object_get_string(obj);
            return 0;
        }


        if(0 == strcmp(key,"es2"))
        {
            json_data_out->es2_ver = json_object_get_string(obj);
            return 0;
        }

        if(0 == strcmp(key,"es3"))
        {
            json_data_out->es3_ver = json_object_get_string(obj);
            return 0;
        }

        if(0 == strcmp(key,"es4"))
        {
            json_data_out->es4_ver = json_object_get_string(obj);
            return 0;
        }

        if(0 == strcmp(key,"url"))
        {
            json_data_out->url = json_object_get_string(obj);
            return 0;
        }

        if(0 == strcmp(key,"md5"))
        {
            json_data_out->md5 = json_object_get_string(obj);
            return 0;
        }

        if(0 == strcmp(key,"disable"))
        {
            json_data_out->disable = json_object_get_string(obj);
            return 0;
        }

    }
    else if(type == json_type_object)
    {
        json_loop_object(obj,json_data_out);
    }
    else
    {
        return -2;
    }

    return 0;

}


int CAgvJson::DecodeStringFromServer(const char *json_data_in,JsonDataOut *json_data_out)
{
    json = json_tokener_parse(json_data_in);

    if(json_get_value(json,NULL,json_data_out) != 0)
    {
        return -1;
    }


    return 0;
}

const char* CAgvJson::EncodeStringToServer(char *pid,char *es0_ver,char *es1_ver,char *es2_ver,char *es3_ver,char *es4_ver,char *dis)
{
    json = json_object_new_object();
    json_object_object_add(json,"pid",json_object_new_string(pid));
    json_object_object_add(json,"es0",json_object_new_string(es0_ver));
    json_object_object_add(json,"es1",json_object_new_string(es1_ver));
    json_object_object_add(json,"es2",json_object_new_string(es2_ver));
    json_object_object_add(json,"es3",json_object_new_string(es3_ver));
    json_object_object_add(json,"es4",json_object_new_string(es4_ver));
    json_object_object_add(json,"disable",json_object_new_string(dis));

    const char *str=json_object_to_json_string(json);//json



    return str;
}
