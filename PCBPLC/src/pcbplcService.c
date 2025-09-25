#include <stdlib.h>
//#include <dirent.h>
//#include <unistd.h>
#include <memory.h>
#include <malloc.h>
#include <sys/time.h>

#include "pcbplcInterface.h"
#include "pcbplcService.h"
#include "pcbplccomm.h"
#include "pcbplcConfig.h"

pcbplcCnfg_t gpcbplcCnfg;
extern uint8_t pcbplcfile[12*1024];
extern uint8_t RecFile[1*1024];

bool continue_loop = true;
unsigned int lograteTimeSliceDelay_Second=0;
void *timer_thread_routine(void *arg);

static long sampling_rate = 10; //In milli second

bool service_destroy()
{
    return true;
}

bool service_start()
{
	char logBuff[256]= {0, };
	unsigned int new_time = 0, prev_time = 0;

	if ((checkFileAvibility(gPlcFile)==0) && (checkFileAvibility(gRecFile)==0))
	{
        PLC_RPOG_Flag = 0;
        Extract_Alloc();

        if(xSemaphoreTake(sendExternalFlashSemaphore, 5000) == pdTRUE )
		{
			ReadRecipeFile();
			xSemaphoreGive(sendExternalFlashSemaphore);
		}
        //TODO : If don't get semaphoshe than retry
        plcTimer_start();
	}
    else
    {
    	WriteLog(pcbplc_logger, "Could not found PLC Logic", LOG_ERROR);
    	//return false;
    }

	//gpcbplcCnfg.mLogRate = EPROM_General.LogRate;

    while (continue_loop)
    {
    	if((1 == PLC_RPOG_Flag)  && (1 == REC_RPOG_Flag))
    	{
    		PLC_RPOG_Flag = 0;
    		REC_RPOG_Flag = 0;

            Extract_Alloc();
			if(xSemaphoreTake(sendExternalFlashSemaphore, 5000) == pdTRUE )
			{
				ReadRecipeFile();
				xSemaphoreGive(sendExternalFlashSemaphore);
			}
            plcTimer_start();
        }

    	osDelay(500); // 500ms
        read_rtu_datetime();

        if (gpcbplcCnfg.debug)
        {
            memset(&logBuff, 0, sizeof(logBuff));
            sprintf(logBuff, "service state %d\r\n", gpcbplcCnfg.mIsEnable);
            WriteLog(pcbplc_logger, logBuff, LOG_INFO);
        }

       // if(gpcbplcCnfg.mIsEnable == 1)
        {
            get_di_status();
            get_ai_status();
			set_do();
			set_dual_Do();
            //set_ao(0,100);
            //get_rs485_parameters();  // unused

            if (IsLogRateMatched())
            {
                //char tDataBuf[1024] = {0,};
            	Build_Data_for_server();
            	//LoraPublish();
            	flagMqttPubLogData = 1;
            	flagLORAPubLogData = 1;
            	flagLORAPubLogData_fail = 255; // set 255 to check if it failed
//                if (!Build_Data_for_server())
//                {
//                    //send_event_string(tDataBuf);
//                    // maulin use : publish  function
//                }
//                else
//                {
//                    WriteLog(pcbplc_logger, "Data Sent on LogRate failed due to payload is not framed\r\n", LOG_CRITICAL);
//                }
            }
            else
            {
                //char tDataBuf[1024] = {0,};

//                if (gpcbplcCnfg.debug)
//                {
//                    WriteLog(pcbplc_logger, "Data Sent on Every Second for Webscanet Display\r\n", LOG_INFO);
//                }
//
//                if (!Build_Data_for_server(tDataBuf))
//                {
//                    //send_data_string(tDataBuf);
//                	// maulin : call publish function to send live streaming data
//                }
//                else
//                {
//                    WriteLog(pcbplc_logger, "Data Sent on LogRate failed due to payload is not framed\r\n", LOG_CRITICAL);
//                }
            }
            pcbplc_BaseLoop();

            new_time = ((gTimeInfo.mHour * 3600) + (gTimeInfo.mMinute * 60) + gTimeInfo.mSecond);
            if (new_time - prev_time > 10)
            {
                prev_time = new_time;
//                Store_General_purpose_analog_parameter();
//
                if (gpcbplcCnfg.debug)
                {
                    memset(&logBuff, 0, sizeof(logBuff));
                    sprintf(logBuff, "Store general purpose data\r\n");
                    WriteLog(pcbplc_logger, logBuff, LOG_INFO);
                }
            }
        }

//        if(true == gPcbplcInfo.mReloadCnfg)
//        {
//            gPcbplcInfo.mReloadCnfg = false;
//            if(-1 == pcbplc_config_json_parsing(&gpcbplcCnfg))
//            {
//                WriteLog(pcbplc_logger, "Could not found configuration file", LOG_ERROR);
//                break;
//            }
//            pcbplc_config_log(&gpcbplcCnfg);
//
//            if (gpcbplcCnfg.debug)
//            {
//                memset(&logBuff, 0, sizeof(logBuff));
//                sprintf(logBuff, "Found reload_config event");
//                WriteLog(pcbplc_logger, logBuff, LOG_INFO);
//            }
//        }
//
//        if(true == gPcbplcInfo.mReloadReceipe)
//        {
//            gPcbplcInfo.mReloadReceipe = false;
//            WriteModifiedRecipeFile(MODIFIED_RECIPE_FILE_PATH);
//            ReadRecipeJsonFile();
//        }

        osDelay(sampling_rate);  // ms
    }

    return true;
}

bool service_restart()
{
    return false;
}

bool service_stop()
{
    /* Free memory which is allocated for pcbplc extraction */
    pcbplc_memory_free();

//    if (message_bus)
//    {
//        message_bus_close(message_bus);
//        message_bus_release(message_bus);
//    }

//    if (destination_list)
//    {
//        for(int tIdx = 0 ; tIdx < gpcbplcCnfg.mNumOfDest; ++tIdx)
//        {
//            if(destination_list[tIdx])
//            {
//                free(destination_list[tIdx]);
//                destination_list[tIdx] = NULL;
//            }
//        }
//        free(destination_list);
//        destination_list = NULL;
//        //strfreelist(destination_list);
//    }

//    if (logger)
//    {
//        logger_release(logger);
//    }

    return false;
}

void send_data_string(char *pDataBuf)
{
//    if (NULL == pDataBuf)
//    {
//        WriteLog(pcbplc_logger, "pDataBuf is null", LOG_ERROR);
//        return;
//    }
//
//    if (destination_list)
//    {
//        //for (int tIndex = 0; destination_list[tIndex]; ++tIndex)
//        for (int tIndex = 0; tIndex < gpcbplcCnfg.mNumOfDest; ++tIndex)
//        {
//            if (!message_bus_send(message_bus, destination_list[tIndex], Data, Text, (char *) pDataBuf,
//                                  (long) strlen(pDataBuf)))
//            {
//                WriteLog(pcbplc_logger, "Send Data failure over IPC", LOG_ERROR);
//            }
//        }
//    }

    return;
}

void send_event_string(char *pDataBuf)
{
//    if (NULL == pDataBuf)
//    {
//        WriteLog(pcbplc_logger, "pDataBuf is null", LOG_ERROR);
//        return;
//    }
//
//    if (destination_list)
//    {
//        //for (int tIndex = 0; destination_list[tIndex]; ++tIndex)
//        for (int tIndex = 0; tIndex < gpcbplcCnfg.mNumOfDest; ++tIndex)
//        {
//            if (!message_bus_send(message_bus, destination_list[tIndex], Event, Text, (char *) pDataBuf,
//                                  (long) strlen(pDataBuf)))
//            {
//                WriteLog(pcbplc_logger, "Send Data failure over IPC", LOG_ERROR);
//            }
//        }
//    }

    return;
}
void read_rtu_datetime() {

//    time_t t = time(NULL);
//    t += gpcbplcCnfg.mtimeoffset;
//
//    struct tm tm = *gmtime(&t);
//    gTimeInfo.mHour = tm.tm_hour;
//    gTimeInfo.mMinute = tm.tm_min;
//    gTimeInfo.mSecond = tm.tm_sec;
//    gTimeInfo.mDate = tm.tm_mday;
//    gTimeInfo.mMonth = tm.tm_mon + 1;
//    gTimeInfo.mYear = tm.tm_year + 1900;
//    gTimeInfo.mDayofWeek = tm.tm_wday;
//
//    if(gpcbplcCnfg.debug)
//    {
//        char tBuf[128] = {0,};
//        sprintf(tBuf, "rtu date: %02d/%02d/%04d, time: %02d:%02d:%02d ", gTimeInfo.mDate, gTimeInfo.mMonth, gTimeInfo.mYear,
//                gTimeInfo.mHour, gTimeInfo.mMinute, gTimeInfo.mSecond);
//        WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//    }

	gTimeInfo.mHour=gTime.Hours;
	gTimeInfo.mMinute=gTime.Minutes;
	gTimeInfo.mSecond=gTime.Seconds;
	gTimeInfo.mDate=gDate.Date;
	gTimeInfo.mMonth=gDate.Month;
	gTimeInfo.mYear=gDate.Year;
	gTimeInfo.mDayofWeek = gDate.WeekDay;

	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+0] = gTimeInfo.mDate;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+1] = gTimeInfo.mMonth;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+2] = gTimeInfo.mYear + 2000;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+3] = gTimeInfo.mHour;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+4] = gTimeInfo.mMinute;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+5] = gTimeInfo.mSecond;
}

//void timer_thread_start() {
//    pthread_t timer_thread;
//
//    pthread_create(&timer_thread, NULL, timer_thread_routine, NULL);
//
//    return;
//}
//
//void *timer_thread_routine(void *arg) {
//    pthread_detach(pthread_self());
//
//    while (1) {
//        /* TODO : Need to enhance for exact 1 Second delay time */
//        sleep(1);
//        ++plcTimerTicks;
//    }
//
//    pthread_exit(NULL);
//}


int IsLogRateMatched()
{
    int tIsMatched = 0;
    static unsigned char tsOld_minute=255;
    static unsigned char tsIsFirstTime = 1;
    static unsigned char tsOld_hour=255;

    if((EPROM_General.LogRate == 60) || (EPROM_General.LogRate == 120) || (EPROM_General.LogRate == 180) || (EPROM_General.LogRate == 240) || (EPROM_General.LogRate == 360) || (EPROM_General.LogRate == 720))
    {
    	//if((gTimeInfo.mHour != tsOld_hour)||((flagLORAPubLogData_fail == 1)))
    	if( (((gTimeInfo.mHour%(unsigned char)(EPROM_General.LogRate/60)) == 0) && (gTimeInfo.mHour != tsOld_hour))||((flagLORAPubLogData_fail == 1)))
    	{
			if((((lograteTimeSliceDelay_Second)-((gTimeInfo.mMinute * 60) + gTimeInfo.mSecond))<2)
					&&(((lograteTimeSliceDelay_Second)-((gTimeInfo.mMinute * 60) + gTimeInfo.mSecond))>0))
			{
				tsOld_hour = gTimeInfo.mHour;
				tIsMatched = 1;
			}
			else if((((lograteTimeSliceDelay_Second+EPROM_General.maxLograteTimeSliceDelayS)-((gTimeInfo.mMinute * 60) + gTimeInfo.mSecond))<2)
					&&(((lograteTimeSliceDelay_Second+EPROM_General.maxLograteTimeSliceDelayS)-((gTimeInfo.mMinute * 60) + gTimeInfo.mSecond))>0))
			{
				tsOld_hour = gTimeInfo.mHour;
				tIsMatched = 1;
			}
    	}
    }
    else
    {
        if ((((gTimeInfo.mHour * 3600) + (gTimeInfo.mMinute * 60) + gTimeInfo.mSecond) % (EPROM_General.LogRate * 60)) < 2)  // for log rate data structure match to use in IsLogRateMatched() func || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-22
        {
            if((1 == tsIsFirstTime) || (tsOld_minute != gTimeInfo.mMinute))
            {
                tsIsFirstTime = 0;
                tsOld_minute = gTimeInfo.mMinute;
                tsOld_hour = gTimeInfo.mHour;
                tIsMatched = 1;
            }
            else if ( ((EPROM_General.LogRate % 60) == 0) && (tsOld_hour != gTimeInfo.mHour))  // for log rate data structure match to use in IsLogRateMatched() func || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-22
            {
                tsOld_hour = gTimeInfo.mHour;
                tIsMatched = 1;
            }
        }
    }

    return (tIsMatched);
}

//char* ReadJsonFile(const char *pFileName)
//{
//    int nread = 0;
//
//    if(NULL == pFileName)
//    {
//        return NULL;
//    }
//
//    int tFileSize = findSize(pFileName);
//    //printf("tFileSize : %d\n", tFileSize);
//    char *pJsonData = calloc(tFileSize + 10, sizeof(char));
//
//    FILE *tFp = fopen(pFileName, "r");
//
//    if(tFp != NULL)
//    {
//        nread = fread(pJsonData, tFileSize, 1, tFp);
//    }
//
//    fclose(tFp);
//
//    return pJsonData;
//}

//void on_network_event(const char *node_name, PayloadType ptype, DataType dtype, const char *messagebuffer, int32_t buffersize, int32_t payload_id) {
//    //We assume Text and Userdata for data type and message type
//    switch (ptype) {
//        case Data: {
//            switch (dtype) {
//                case Text: {
//                    printf("Text Data Recived: %s\n", messagebuffer);
//                    char request[256]= "";
//                    char *requestdata = NULL;
//
//                    json_object *root_rt = json_tokener_parse(messagebuffer);
//                    if(!root_rt)
//                    {
//                        return;
//                    }
//
//                    json_object *obj_request = json_object_object_get(root_rt, "request");
//                    if(obj_request)
//                    {
//                        char *ptr = (char*)json_object_get_string(obj_request);
//                        if(ptr)
//                        {
//                            strncpy(request, ptr, sizeof(request));
//                        }
//                    }
//
//                    json_object *obj_request_para = json_object_object_get(root_rt, "request_parameters");
//                    if(obj_request_para)
//                    {
//                        char *ptr = (char*)json_object_to_json_string(obj_request_para);
//                        if(ptr)
//                        {
//                            requestdata = (char *)calloc(1, strlen(messagebuffer)* sizeof(char*));
//                            if(requestdata)
//                            {
//                                strcpy(requestdata, ptr);
//                            }
//                        }
//                    }
//
//                    if (strstr(request, "set_schedule"))
//                    {
//                        json_object *obj_schedule_number =  json_object_object_get(obj_request_para, "schedule_num");
//                        if(obj_schedule_number)
//                        {
//                            int schedule_number = json_object_get_int(obj_schedule_number);
//                            if( (schedule_number > 0) && (schedule_number < 10))
//                            {
//                                int schedule_start = 0;
//                                int schedule_end = 0;
//
//                                if (schedule_number == 9)
//                                {
//                                    schedule_end = MAXSCH;
//                                    schedule_start = 80;
//                                }
//                                else
//                                {
//                                    schedule_end = schedule_number * 10;
//                                    schedule_start = schedule_end - 10;
//                                }
//
//                                json_object *obj_set_schedule = json_object_object_get(obj_request_para, "schedule");
//                                if (obj_set_schedule)
//                                {
//                                    int array_length = json_object_array_length(obj_set_schedule);
//                                    if (array_length <= 10)
//                                    {
//                                        configuration_create_backup(NULL);
//                                        char *filename = (char *) configuration_get_filepath();
//                                        if (filename)
//                                        {
//                                            json_object *parse_result = json_object_from_file(filename);
//                                            if (parse_result)
//                                            {
//                                                json_object *file_object = json_object_object_get(parse_result, "schedule");
//
//                                                int replace_index = schedule_start + array_length;
//                                                int index = 0;
//                                                for ( ; schedule_start < schedule_end; schedule_start++)
//                                                {
//                                                    json_object *sch = NULL;
//                                                    json_object *temp = NULL;
//                                                    if(array_length > index)
//                                                    {
//                                                        sch = json_object_array_get_idx(obj_set_schedule,index);
//                                                        json_object_deep_copy(sch, &temp, NULL);
//                                                        index++;
//                                                    }
//                                                    if (replace_index > schedule_start)
//                                                    {
//                                                        json_object_array_put_idx(file_object, schedule_start, temp);
//                                                    }
//                                                    else
//                                                    {
//                                                        json_object *add_sch = json_object_new_object();
//                                                        json_object_object_add(add_sch, "enable", json_object_new_int(0));
//                                                        json_object_object_add(add_sch, "start_hour",json_object_new_int(0));
//                                                        json_object_object_add(add_sch, "start_minute",json_object_new_int(0));
//                                                        json_object_object_add(add_sch, "stop_hour",json_object_new_int(0));
//                                                        json_object_object_add(add_sch, "stop_minute",json_object_new_int(0));
//                                                        json_object_array_put_idx(file_object, schedule_start, add_sch);
//                                                    }
//                                                }
//
//                                                int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                                if (d > -1)
//                                                {
//                                                    json_object_to_fd(d, parse_result, JSON_C_TO_STRING_PRETTY |JSON_C_TO_STRING_NOSLASHESCAPE);
//                                                    close(d);
//                                                }
//                                                json_object_put(parse_result);
//                                                gPcbplcInfo.mReloadCnfg = true;
//                                            }
//                                            free(filename);
//                                            filename = NULL;
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                    else if (strstr(request, "get_schedule"))
//                    {
//                        json_object *obj_get_schedule =  json_object_object_get(obj_request_para, "schedule_num");
//
//                        if(obj_get_schedule)
//                        {
//                            int schedule_num = json_object_get_int(obj_get_schedule);
//                            if( (schedule_num > 0) && (schedule_num < 10) )
//                            {
//                                int schedule_end = 0;
//                                int schedule_start = 0;
//
//                                if (schedule_num == 9)
//                                {
//                                    schedule_end = MAXSCH;
//                                    schedule_start = 80;
//                                }
//                                else
//                                {
//                                    schedule_end = schedule_num * 10;
//                                    schedule_start = schedule_end - 10;
//                                }
//
//
//                                json_object *jobj_main = json_object_new_object();
//                                json_object *jobj_Para_array = json_object_new_array();
//
//                                json_object_object_add(jobj_main, "client_id",json_object_new_int(gpcbplcCnfg.mClientAddr));
//                                json_object_object_add(jobj_main, "group_id",json_object_new_int(gpcbplcCnfg.mGroupAddr));
//                                json_object_object_add(jobj_main, "rtu_id", json_object_new_int(gpcbplcCnfg.mRtuAddr));
//                                json_object_object_add(jobj_main, "schedule_num", json_object_new_int(schedule_num));
//
//                                for (; schedule_start < schedule_end; schedule_start++)
//                                {
//                                    json_object *jobj = json_object_new_object();
//                                    if (schedule_start < gpcbplcCnfg.mMaxSchEnabled)
//                                    {
//                                        json_object_object_add(jobj, "enable", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mIsSchEnabled));
//                                        json_object_object_add(jobj, "start_hour", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStartHour));
//                                        json_object_object_add(jobj, "start_minute", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStartMin));
//                                        json_object_object_add(jobj, "stop_hour", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStopHour));
//                                        json_object_object_add(jobj, "stop_minute", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStopMin));
//                                        json_object_array_add(jobj_Para_array, jobj);
//                                    }
//                                    else
//                                    {
//                                        json_object_object_add(jobj, "enable", json_object_new_int(0));
//                                        json_object_object_add(jobj, "start_hour", json_object_new_int(0));
//                                        json_object_object_add(jobj, "start_minute", json_object_new_int(0));
//                                        json_object_object_add(jobj, "stop_hour", json_object_new_int(0));
//                                        json_object_object_add(jobj, "stop_minute", json_object_new_int(0));
//                                        json_object_array_add(jobj_Para_array, jobj);
//                                    }
//                                }
//                                json_object_object_add(jobj_main, "schedule", jobj_Para_array);
//                                if (!message_bus_send(message_bus, node_name, Response, Text,
//                                                      (char *) json_object_to_json_string(jobj_main),strlen(json_object_to_json_string(jobj_main)))) {
//                                    WriteLog(pcbplc_logger, "Send Data failure over IPC", LOG_ERROR);
//                                }
//                                json_object_put(jobj_main);
//                            }
//                        }
//                    }
//                    else if (strstr(request, "set_do_key_status"))
//                    {
//                        json_object *obj_do_key =json_object_object_get(obj_request_para, "do_key_data");
//                        if(obj_do_key != NULL)
//                        {
//                            int pin_no = -1, pin_value = -1;
//
//                            json_object *obj_pin_no =json_object_object_get(obj_do_key, "pin-no");
//                            if(obj_pin_no)
//                            {
//                                pin_no = json_object_get_int(obj_pin_no);
//                            }
//
//                            json_object *obj_pin_value =json_object_object_get(obj_do_key, "value");
//                            if(obj_pin_value)
//                            {
//                                pin_value = json_object_get_int(obj_pin_value);
//                            }
//
//                            if(0 != update_do_status_key(pin_no, pin_value))
//                            {
//                                WriteLog(pcbplc_logger, "Do key update failed", LOG_ERROR);
//                            }
//                            else
//                            {
//                                gPcbplcInfo.mReloadCnfg = true;
//                            }
//
//                            #if 0
//                            if( (gpcbplcCnfg.mMaxDoEnabled >= pin_no) && (pin_no != 0))
//                            {
//                                configuration_create_backup(NULL);
//
//                                char *filename = (char *) configuration_get_filepath();
//                                if (filename)
//                                {
//                                    json_object *parse_result = json_object_from_file(filename);
//                                    if (parse_result)
//                                    {
//
//                                        json_object *do_Status = json_object_object_get(parse_result, "do_status");
//                                        for (int i = 0; i < pin_no; i++)
//                                        {
//                                            if (pin_no - 1 == i)
//                                            {
//                                                json_object_array_put_idx(do_Status, i, json_object_new_int(pin_value));
//                                                break;
//                                            }
//                                        }
//
//                                        int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                        if (d > -1)
//                                        {
//                                            json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                                            close(d);
//                                        }
//                                        free(filename);
//                                        filename = NULL;
//                                        json_object_put(parse_result);
//                                        gPcbplcInfo.mReloadCnfg = true;
//                                    }
//                                }
//                                else
//                                {
//                                    WriteLog(pcbplc_logger, "configuration file not found", LOG_CRITICAL);
//                                }
//                            }
//                            #endif
//                        }
//                    }
//                    else if (strstr(request, "get_mode"))
//                    {
//                        json_object * mod_obj   = json_object_new_object();
//
//                        if(mod_obj)
//                        {
//                            json_object_object_add(mod_obj, "client_id", json_object_new_int(gpcbplcCnfg.mClientAddr));
//                            json_object_object_add(mod_obj, "group_id", json_object_new_int(gpcbplcCnfg.mGroupAddr));
//                            json_object_object_add(mod_obj, "rtu_id", json_object_new_int(gpcbplcCnfg.mRtuAddr));
//                            json_object_object_add(mod_obj, "pcbplc_mode", json_object_new_int(gpcbplcCnfg.mRtuDoMode));
//                            message_bus_send(message_bus, node_name, Response, Text,
//                                             (char *) json_object_to_json_string(mod_obj),
//                                             strlen(json_object_to_json_string(mod_obj)));
//                            if (gpcbplcCnfg.debug)
//                            {
//                                WriteLog(pcbplc_logger, (char *) json_object_to_json_string(mod_obj), LOG_INFO);
//                            }
//                            json_object_put(mod_obj);
//                        }
//                    }
//                    else if (strstr(request, "set_mode"))
//                    {
//                        json_object *json_mode =  json_object_object_get(obj_request_para, "pcbplc_mode");
//                        if(json_mode)
//                        {
//                            int mode = json_object_get_int(json_mode);
//
//                            configuration_create_backup(NULL);
//
//                            char *filename = (char *) configuration_get_filepath();
//                            if (filename)
//                            {
//                                json_object *parse_result = json_object_from_file(filename);
//                                if (parse_result)
//                                {
//                                    json_object_object_add(parse_result, "mode", json_object_new_int(mode));
//                                    int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                    if (d > -1)
//                                    {
//                                        json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                                        close(d);
//                                    }
//                                    gPcbplcInfo.mReloadCnfg = true;
//                                }
//                                free(filename);
//                                filename = NULL;
//                            }
//                        }
//                    }
//                    if(requestdata)
//                    {
//                        free(requestdata);
//                        requestdata = NULL;
//                    }
//
//                    if(root_rt)
//                    {
//                        json_object_put(root_rt);
//                        root_rt = NULL;
//                    }
//                    break;
//                }
//                case Video: {
//                    break;
//                }
//                case Image: {
//                    break;
//                }
//                case Audio: {
//                    break;
//                }
//                case Raw: {
//                    break;
//                }
//            }
//            break;
//        }
//        case Event: {
//            printf("Event Recived: %s\n", messagebuffer);
//            break;
//        }
//        case Request:
//        {
//            //printf("Request Received: %s\n", messagebuffer);
//            if(dtype == Text)
//            {
//                char request[256] = "";
//                char *requestdata = NULL;
//
//                json_object *root_rt = json_tokener_parse(messagebuffer);
//                if(!root_rt)
//                {
//                    return;
//                }
//
//                json_object *obj_request = json_object_object_get(root_rt, "request");
//                if(obj_request)
//                {
//                    char *ptr = (char*)json_object_get_string(obj_request);
//                    if(ptr)
//                    {
//                        strncpy(request, ptr, sizeof(request));
//                    }
//                }
//
//                json_object *obj_request_para = json_object_object_get(root_rt, "request_parameters");
//                if(obj_request_para)
//                {
//                    char *ptr = (char*)json_object_to_json_string(obj_request_para);
//                    if(ptr)
//                    {
//                        requestdata = (char *)calloc(1, strlen(messagebuffer)* sizeof(char*));
//                        if(requestdata)
//                        {
//                            strcpy(requestdata, ptr);
//                        }
//                    }
//                }
//
//                if (strcmp(request, "set_config") == 0)
//                {
//                    configuration_create_backup(NULL);
//
//                    char* filename = (char*)configuration_get_filepath();
//                    if(filename)
//                    {
//                        int d = open(filename,O_WRONLY|O_CREAT|O_TRUNC, 0600);
//                        if(d > -1)
//                        {
//                            json_object_to_fd(d, obj_request_para, JSON_C_TO_STRING_PRETTY| JSON_C_TO_STRING_NOSLASHESCAPE);
//                            close(d);
//                        }
//                        free(filename);
//                        filename = NULL;
//                    }
//                    gPcbplcInfo.mReloadCnfg = true;
//
//                    network_reply(node_name);
//                }
//                else if (strcmp(request, "get_config") == 0)
//                {
//                    char* filename = (char*)configuration_get_filepath();
//                    if(filename)
//                    {
//                        json_object *json_config = json_object_from_file(filename);
//                        if(json_config)
//                        {
//                            message_bus_send(message_bus, node_name, Response, Text,
//                                             (char *) json_object_to_json_string(json_config), strlen(json_object_to_json_string(json_config)));
//
//                            if (gpcbplcCnfg.debug)
//                            {
//                                WriteLog(pcbplc_logger, (char *) json_object_to_json_string(json_config), LOG_INFO);
//                            }
//
//                            json_object_put(json_config);
//                        }
//                        free(filename);
//                        filename = NULL;
//                    }
//                    else
//                    {
//                        WriteLog(pcbplc_logger, "configuration file not found", LOG_CRITICAL);
//                    }
//
//                    #if 0
//                    char *jsondata = ReadConfigFromJson();
//                    buffer_t *payload = buffer_allocate_default();
//
//                    buffer_append_char(payload, '{');
//                    buffer_append_string(payload, "\"timestamp\":");
//                    buffer_append_unix_timestamp_ms(payload);
//                    buffer_append_char(payload, ',');
//
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "status");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_string(payload, "\"success\"");
//                    buffer_append_char(payload, ',');
//
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "response_code");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_integer(payload, 0);
//                    buffer_append_char(payload, ',');
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "response_message");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_string(payload, jsondata);
//                    buffer_append_char(payload, '}');
//
//                    printf("response: %s\n", (char *) buffer_get_data(payload));
//                    message_bus_send(message_bus, node_name, Response, Text, (char *) buffer_get_data(payload),
//                                     buffer_get_size(payload));
//                    if(payload) {
//                        free(payload);
//                    }
//                    if(jsondata) {
//                        free(jsondata);
//                    }
//                    #endif
//                }
//                /*
//                else if (strcmp(request, "set_data") == 0)
//                {
//                    FILE *fp = fopen(RECIPE_JSON_FILE_PATH , "w+");
//                    fputs(requestdata, fp);
//                    fclose(fp);
//
//                    gPcbplcInfo.mReloadReceipe = true;
//
//                    buffer_t *payload = buffer_allocate_default();
//
//                    buffer_append_char(payload, '{');
//                    buffer_append_string(payload, "\"timestamp\":");
//                    buffer_append_unix_timestamp_ms(payload);
//                    buffer_append_char(payload, ',');
//
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "status");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_string(payload, "\"success\"");
//                    buffer_append_char(payload, ',');
//
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "response_code");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_integer(payload, 0);
//                    buffer_append_char(payload, ',');
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "response_message");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_string(payload, "success");
//                    buffer_append_char(payload, '}');
//
//                    printf("response: %s\n", (char *) buffer_get_data(payload));
//                    message_bus_send(message_bus, node_name, Response, Text, (char *) buffer_get_data(payload),
//                                     buffer_get_size(payload));
//                    if(payload) {
//                        free(payload);
//                        payload = NULL;
//                    }
//                }
//                else if (strcmp(request, "get_data") == 0)
//                {
//                    char *pJsonData = ReadJsonFile(RECIPE_JSON_FILE_PATH);
//                    buffer_t *payload = buffer_allocate_default();
//
//                    buffer_append_char(payload, '{');
//                    buffer_append_string(payload, "\"timestamp\":");
//                    buffer_append_unix_timestamp_ms(payload);
//                    buffer_append_char(payload, ',');
//
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "status");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_string(payload, "\"success\"");
//                    buffer_append_char(payload, ',');
//
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "response_code");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_integer(payload, 0);
//                    buffer_append_char(payload, ',');
//                    buffer_append_char(payload, '"');
//                    buffer_append_string(payload, "response_message");
//                    buffer_append_char(payload, '"');
//                    buffer_append_char(payload, ':');
//                    buffer_append_string(payload, pJsonData);
//                    buffer_append_char(payload, '}');
//
//                    printf("response: %s\n", (char *) buffer_get_data(payload));
//                    message_bus_send(message_bus, node_name, Response, Text, (char *) buffer_get_data(payload),
//                                     buffer_get_size(payload));
//                    if(payload) {
//                        free(payload);
//                        payload = NULL;
//                    }
//                    if(pJsonData) {
//                        free(pJsonData);
//                        pJsonData = NULL;
//                    }
//                }
//                 */
//                else if (strcmp(request, "set_schedule") == 0)
//                {
//                    json_object *obj_schedule_number =  json_object_object_get(obj_request_para, "schedule_num");
//                    if(obj_schedule_number)
//                    {
//                        int schedule_number = json_object_get_int(obj_schedule_number);
//                        if( (schedule_number > 0) && (schedule_number < 10))
//                        {
//                            int schedule_start = 0;
//                            int schedule_end = 0;
//
//                            if (schedule_number == 9)
//                            {
//                                schedule_end = MAXSCH;
//                                schedule_start = 80;
//                            }
//                            else
//                            {
//                                schedule_end = schedule_number * 10;
//                                schedule_start = schedule_end - 10;
//                            }
//
//                            json_object *obj_set_schedule = json_object_object_get(obj_request_para, "schedule");
//                            if (obj_set_schedule)
//                            {
//                                int array_length = json_object_array_length(obj_set_schedule);
//                                if (array_length <= 10)
//                                {
//                                    configuration_create_backup(NULL);
//                                    char *filename = (char *) configuration_get_filepath();
//                                    if (filename)
//                                    {
//                                        json_object *parse_result = json_object_from_file(filename);
//                                        if (parse_result)
//                                        {
//                                            json_object *file_object = json_object_object_get(parse_result, "schedule");
//
//                                            int replace_index = schedule_start + array_length;
//                                            int index = 0;
//                                            for ( ; schedule_start < schedule_end; schedule_start++)
//                                            {
//                                                json_object *sch = NULL;
//                                                json_object *temp = NULL;
//                                                if(array_length > index)
//                                                {
//                                                    sch = json_object_array_get_idx(obj_set_schedule,index);
//                                                    json_object_deep_copy(sch, &temp, NULL);
//                                                    index++;
//                                                }
//                                                if (replace_index > schedule_start)
//                                                {
//                                                    json_object_array_put_idx(file_object, schedule_start, temp);
//                                                }
//                                                else
//                                                {
//                                                    json_object *add_sch = json_object_new_object();
//                                                    json_object_object_add(add_sch, "enable", json_object_new_int(0));
//                                                    json_object_object_add(add_sch, "start_hour",json_object_new_int(0));
//                                                    json_object_object_add(add_sch, "start_minute",json_object_new_int(0));
//                                                    json_object_object_add(add_sch, "stop_hour",json_object_new_int(0));
//                                                    json_object_object_add(add_sch, "stop_minute",json_object_new_int(0));
//                                                    json_object_array_put_idx(file_object, schedule_start, add_sch);
//                                                }
//                                            }
//
//                                            int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                            if (d > -1)
//                                            {
//                                                json_object_to_fd(d, parse_result, JSON_C_TO_STRING_PRETTY |JSON_C_TO_STRING_NOSLASHESCAPE);
//                                                close(d);
//                                            }
//                                            json_object_put(parse_result);
//                                            gPcbplcInfo.mReloadCnfg = true;
//                                        }
//                                        free(filename);
//                                        filename = NULL;
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//                else if (strcmp(request, "get_schedule") == 0)
//                {
//                    json_object *obj_get_schedule =  json_object_object_get(obj_request_para, "schedule_num");
//
//                    if(obj_get_schedule)
//                    {
//                        int schedule_num = json_object_get_int(obj_get_schedule);
//                        if( (schedule_num > 0) && (schedule_num < 10) )
//                        {
//                            int schedule_end = 0;
//                            int schedule_start = 0;
//
//                            if (schedule_num == 9)
//                            {
//                                schedule_end = MAXSCH;
//                                schedule_start = 80;
//                            }
//                            else
//                            {
//                                schedule_end = schedule_num * 10;
//                                schedule_start = schedule_end - 10;
//                            }
//
//                            json_object *jobj_main = json_object_new_object();
//                            json_object *jobj_Para_array = json_object_new_array();
//
//                            json_object_object_add(jobj_main, "client_id",json_object_new_int(gpcbplcCnfg.mClientAddr));
//                            json_object_object_add(jobj_main, "group_id",json_object_new_int(gpcbplcCnfg.mGroupAddr));
//                            json_object_object_add(jobj_main, "rtu_id", json_object_new_int(gpcbplcCnfg.mRtuAddr));
//                            json_object_object_add(jobj_main, "schedule_num", json_object_new_int(schedule_num));
//
//                            for (; schedule_start < schedule_end; schedule_start++)
//                            {
//                                json_object *jobj = json_object_new_object();
//                                if (schedule_start < gpcbplcCnfg.mMaxSchEnabled)
//                                {
//                                    json_object_object_add(jobj, "enable", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mIsSchEnabled));
//                                    json_object_object_add(jobj, "start_hour", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStartHour));
//                                    json_object_object_add(jobj, "start_minute", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStartMin));
//                                    json_object_object_add(jobj, "stop_hour", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStopHour));
//                                    json_object_object_add(jobj, "stop_minute", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStopMin));
//                                    json_object_array_add(jobj_Para_array, jobj);
//                                }
//                                else
//                                {
//                                    json_object_object_add(jobj, "enable", json_object_new_int(0));
//                                    json_object_object_add(jobj, "start_hour", json_object_new_int(0));
//                                    json_object_object_add(jobj, "start_minute", json_object_new_int(0));
//                                    json_object_object_add(jobj, "stop_hour", json_object_new_int(0));
//                                    json_object_object_add(jobj, "stop_minute", json_object_new_int(0));
//                                    json_object_array_add(jobj_Para_array, jobj);
//                                }
//                            }
//                            json_object_object_add(jobj_main, "schedule", jobj_Para_array);
//                            if (!message_bus_send(message_bus, node_name, Response, Text,
//                                                  (char *) json_object_to_json_string(jobj_main),strlen(json_object_to_json_string(jobj_main)))) {
//                                WriteLog(pcbplc_logger, "Send Data failure over IPC", LOG_ERROR);
//                            }
//                            json_object_put(jobj_main);
//                        }
//                    }
//                }
//                else if (strcmp(request, "set_do_key_status") == 0)
//                {
//                    json_object *obj_do_key =json_object_object_get(obj_request_para, "do_key_data");
//                    if(obj_do_key != NULL)
//                    {
//                        int pin_no = -1, pin_value = -1;
//
//                        json_object *obj_pin_no =json_object_object_get(obj_do_key, "pin-no");
//                        if(obj_pin_no)
//                        {
//                            pin_no = json_object_get_int(obj_pin_no);
//                        }
//
//                        json_object *obj_pin_value =json_object_object_get(obj_do_key, "value");
//                        if(obj_pin_value)
//                        {
//                            pin_value = json_object_get_int(obj_pin_value);
//                        }
//
//                        if( (gpcbplcCnfg.mMaxDoEnabled >= pin_no) && (pin_no != 0))
//                        {
//                            configuration_create_backup(NULL);
//
//                            char *filename = (char *) configuration_get_filepath();
//                            if (filename)
//                            {
//                                json_object *parse_result = json_object_from_file(filename);
//                                if (parse_result)
//                                {
//
//                                    json_object *do_Status = json_object_object_get(parse_result, "do_status");
//                                    for (int i = 0; i < pin_no; i++)
//                                    {
//                                        if (pin_no - 1 == i)
//                                        {
//                                            json_object_array_put_idx(do_Status, i, json_object_new_int(pin_value));
//                                            break;
//                                        }
//                                    }
//
//                                    int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                    if (d > -1)
//                                    {
//                                        json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                                        close(d);
//                                    }
//                                    free(filename);
//                                    filename = NULL;
//                                    json_object_put(parse_result);
//                                    gPcbplcInfo.mReloadCnfg = true;
//                                }
//                            }
//                            else
//                            {
//                                WriteLog(pcbplc_logger, "configuration file not found", LOG_CRITICAL);
//                            }
//                        }
//                    }
//                }
//                else if (strcmp(request, "get_mode") == 0)
//                {
//                    json_object * mod_obj   = json_object_new_object();
//
//                    if(mod_obj)
//                    {
//                        json_object_object_add(mod_obj, "client_id", json_object_new_int(gpcbplcCnfg.mClientAddr));
//                        json_object_object_add(mod_obj, "group_id", json_object_new_int(gpcbplcCnfg.mGroupAddr));
//                        json_object_object_add(mod_obj, "rtu_id", json_object_new_int(gpcbplcCnfg.mRtuAddr));
//                        json_object_object_add(mod_obj, "pcbplc_mode", json_object_new_int(gpcbplcCnfg.mRtuDoMode));
//                        message_bus_send(message_bus, node_name, Response, Text,
//                                         (char *) json_object_to_json_string(mod_obj),
//                                         strlen(json_object_to_json_string(mod_obj)));
//                        if (gpcbplcCnfg.debug)
//                        {
//                            WriteLog(pcbplc_logger, (char *) json_object_to_json_string(mod_obj), LOG_INFO);
//                        }
//                        json_object_put(mod_obj);
//                    }
//                }
//                else if (strcmp(request, "set_mode") == 0)
//                {
//                    json_object *json_mode =  json_object_object_get(obj_request_para, "pcbplc_mode");
//                    if(json_mode)
//                    {
//                        int mode = json_object_get_int(json_mode);
//
//                        configuration_create_backup(NULL);
//
//                        char *filename = (char *) configuration_get_filepath();
//                        if (filename)
//                        {
//                            json_object *parse_result = json_object_from_file(filename);
//                            if (parse_result)
//                            {
//                                json_object_object_add(parse_result, "mode", json_object_new_int(mode));
//                                int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                if (d > -1)
//                                {
//                                    json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                                    close(d);
//                                }
//                                gPcbplcInfo.mReloadCnfg = true;
//                            }
//                            free(filename);
//                            filename = NULL;
//                        }
//                    }
//                }
//                else if (strcmp(request, "get_schedule_data") == 0)
//                {
//                    int schedule_start = 0;
//                    json_object *jobj_main = json_object_new_object();
//                    json_object *jobj_Para_array = json_object_new_array();
//
//                    for (; schedule_start < gpcbplcCnfg.mMaxSchEnabled; schedule_start++)
//                    {
//                        json_object *jobj = json_object_new_object();
//
//                        json_object_object_add(jobj, "enable", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mIsSchEnabled));
//                        json_object_object_add(jobj, "start_hour", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStartHour));
//                        json_object_object_add(jobj, "start_minute", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStartMin));
//                        json_object_object_add(jobj, "stop_hour", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStopHour));
//                        json_object_object_add(jobj, "stop_minute", json_object_new_int(gpcbplcCnfg.sSch[schedule_start].mStopMin));
//                        json_object_array_add(jobj_Para_array, jobj);
//                    }
//
//                    json_object_object_add(jobj_main, "schedule", jobj_Para_array);
//                    if (!message_bus_send(message_bus, node_name, Response, Text,
//                                          (char *) json_object_to_json_string(jobj_main),strlen(json_object_to_json_string(jobj_main)))) {
//                        WriteLog(pcbplc_logger, "Send Data failure over IPC", LOG_ERROR);
//                    }
//                    json_object_put(jobj_main);
//                }
//                else if (strcmp(request, "set_schedule_data") == 0)
//                {
//                    json_object *obj_set_schedule = json_object_object_get(obj_request_para, "schedule");
//                    if (obj_set_schedule)
//                    {
//                        int array_length = json_object_array_length(obj_set_schedule);
//                        if ( (array_length >= 0) && (array_length < 84) )
//                        {
//                            configuration_create_backup(NULL);
//                            char *filename = (char *) configuration_get_filepath();
//                            if (filename)
//                            {
//                                json_object *parse_result = json_object_from_file(filename);
//                                if (parse_result)
//                                {
//                                    json_object *file_object = json_object_object_get(parse_result, "schedule");
//                                    int file_array_length = json_object_array_length(file_object);
//                                    int index = 0;
//                                    int schedule_end = 0;
//
//                                    if(array_length > 0)
//                                    {
//                                        int devider = array_length % 10;
//                                        if (devider != 0)
//                                        {
//                                            schedule_end = array_length + (10 - devider);
//                                        }
//                                        else
//                                        {
//                                            schedule_end = array_length;
//                                        }
//
//                                        if(schedule_end > 84)
//                                        {
//                                            schedule_end = 84;
//                                        }
//
//                                        for ( ; index < schedule_end; index++)
//                                        {
//                                            json_object *sch = NULL;
//                                            json_object *temp = NULL;
//                                            if(array_length > index)
//                                            {
//                                                sch = json_object_array_get_idx(obj_set_schedule,index);
//                                                json_object_deep_copy(sch, &temp, NULL);
//                                                json_object_array_put_idx(file_object, index, temp);
//                                            }
//                                            else
//                                            {
//                                                json_object *add_sch = json_object_new_object();
//                                                json_object_object_add(add_sch, "enable", json_object_new_int(0));
//                                                json_object_object_add(add_sch, "start_hour",json_object_new_int(0));
//                                                json_object_object_add(add_sch, "start_minute",json_object_new_int(0));
//                                                json_object_object_add(add_sch, "stop_hour",json_object_new_int(0));
//                                                json_object_object_add(add_sch, "stop_minute",json_object_new_int(0));
//                                                json_object_array_put_idx(file_object, index, add_sch);
//                                            }
//                                        }
//
//                                        for (; file_array_length > schedule_end  ; file_array_length--)
//                                        {
//                                            json_object_array_del_idx(file_object, file_array_length-1, 1);
//                                        }
//                                    }
//                                    else
//                                    {
//                                        json_object *temp = NULL;
//                                        json_object_deep_copy(obj_set_schedule, &temp, NULL);
//                                        json_object_object_add(parse_result, "schedule", temp);
//                                    }
//
//
//                                    int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                    if (d > -1)
//                                    {
//                                        json_object_to_fd(d, parse_result, JSON_C_TO_STRING_PRETTY |JSON_C_TO_STRING_NOSLASHESCAPE);
//                                        close(d);
//                                    }
//                                    json_object_put(parse_result);
//                                    gPcbplcInfo.mReloadCnfg = true;
//
//                                    network_reply(node_name);
//                                }
//                                free(filename);
//                                filename = NULL;
//                            }
//                        }
//                        else
//                        {
//                            struct timeval tv;
//                            gettimeofday(&tv, NULL);
//
//                            uint64_t millisecondsSinceEpoch =
//                                    (uint64_t) (tv.tv_sec) * 1000 +
//                                    (uint64_t) (tv.tv_usec) / 1000;
//
//                            json_object * jobj_reply   = json_object_new_object();
//                            json_object_object_add(jobj_reply, "timestamp" ,
//                                                   json_object_new_int64(millisecondsSinceEpoch));
//                            json_object_object_add(jobj_reply, "status", json_object_new_string("failed"));
//                            json_object_object_add(jobj_reply, "response_code", json_object_new_int(-1));
//                            json_object_object_add(jobj_reply, "response_message", json_object_new_string("Max array limit exceed"));
//
//                            message_bus_send(message_bus, node_name, Response, Text,
//                                             (char *) json_object_to_json_string(jobj_reply),strlen(json_object_to_json_string(jobj_reply)) );
//                            json_object_put(jobj_reply);
//                        }
//                    }
//                }
//                else if (strcmp(request, "get_alarm_data") == 0)
//                {
//                    int index = 0;
//                    json_object *jobj_main = json_object_new_object();
//                    json_object *jobj_Para_array = json_object_new_array();
//
//                    for (; index < gpcbplcCnfg.mMaxAlarmEnabled; index++)
//                    {
//                        json_object *jobj = json_object_new_object();
//
//                        json_object_object_add(jobj, "type", json_object_new_int(gpcbplcCnfg.mAlarm[index].mType));
//                        json_object_object_add(jobj, "mobileno", json_object_new_string(gpcbplcCnfg.mAlarm[index].mobileNo));
//                        json_object_array_add(jobj_Para_array, jobj);
//                    }
//
//                    json_object_object_add(jobj_main, "smsinfo", jobj_Para_array);
//                    if (!message_bus_send(message_bus, node_name, Response, Text,
//                                          (char *) json_object_to_json_string(jobj_main),strlen(json_object_to_json_string(jobj_main)))) {
//                        WriteLog(pcbplc_logger, "Send Data failure over IPC", LOG_ERROR);
//                    }
//                    json_object_put(jobj_main);
//                }
//                else if (strcmp(request, "set_alarm_data") == 0)
//                {
//                    json_object *json_alarm =  json_object_object_get(obj_request_para, "smsinfo");
//                    if(json_alarm)
//                    {
//                        configuration_create_backup(NULL);
//
//                        char *filename = (char *) configuration_get_filepath();
//                        if (filename)
//                        {
//                            json_object *parse_result = json_object_from_file(filename);
//                            if (parse_result)
//                            {
//                                json_object *temp = NULL;
//                                json_object_deep_copy(json_alarm, &temp, NULL);
//                                json_object_object_add(parse_result, "smsinfo", temp);
//                                int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                if (d > -1)
//                                {
//                                    json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                                    close(d);
//                                }
//                                json_object_put(parse_result);
//                                gPcbplcInfo.mReloadCnfg = true;
//
//                                network_reply(node_name);
//                            }
//                            free(filename);
//                            filename = NULL;
//                        }
//                    }
//                }
//                else if (strcmp(request, "get_modbus_data") == 0)
//                {
//                    int index = 0;
//                    json_object *jobj_main = json_object_new_object();
//                    json_object *jobj_Para_array = json_object_new_array();
//
//                    for (; index < gpcbplcCnfg.mNumOfModbusConfig; index++)
//                    {
//                        json_object *jobj = json_object_new_object();
//
//                        json_object_object_add(jobj, "device_name", json_object_new_string(gpcbplcCnfg.modbusConfig[index].mDeviceName));
//                        json_object_object_add(jobj, "para_name", json_object_new_string(gpcbplcCnfg.modbusConfig[index].mParaName));
//                        json_object_array_add(jobj_Para_array, jobj);
//                    }
//
//                    json_object_object_add(jobj_main, "modbus", jobj_Para_array);
//                    if (!message_bus_send(message_bus, node_name, Response, Text,
//                                          (char *) json_object_to_json_string(jobj_main),strlen(json_object_to_json_string(jobj_main)))) {
//                        WriteLog(pcbplc_logger, "Send Data failure over IPC", LOG_ERROR);
//                    }
//                    json_object_put(jobj_main);
//                }
//                else if (strcmp(request, "set_modbus_data") == 0)
//                {
//                    json_object *json_modbus =  json_object_object_get(obj_request_para, "modbus");
//                    if(json_modbus)
//                    {
//                        configuration_create_backup(NULL);
//
//                        char *filename = (char *) configuration_get_filepath();
//                        if (filename)
//                        {
//                            json_object *parse_result = json_object_from_file(filename);
//                            if (parse_result)
//                            {
//                                json_object *temp = NULL;
//                                json_object_deep_copy(json_modbus, &temp, NULL);
//                                json_object_object_add(parse_result, "modbus", temp);
//                                int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                                if (d > -1)
//                                {
//                                    json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                                    close(d);
//                                }
//                                json_object_put(parse_result);
//                                gPcbplcInfo.mReloadCnfg = true;
//
//                                network_reply(node_name);
//                            }
//                            free(filename);
//                            filename = NULL;
//                        }
//                    }
//                }
//                else if (strcmp(request, "get_analog_config") == 0)
//                {
//                    char filename[33] = ANALOG_JSON_FILE_PATH;
//                    if(filename)
//                    {
//                        json_object *json_config = json_object_from_file(filename);
//                        if(json_config)
//                        {
//                            message_bus_send(message_bus, node_name, Response, Text,
//                                             (char *) json_object_to_json_string(json_config), strlen(json_object_to_json_string(json_config)));
//
//                            if (gpcbplcCnfg.debug)
//                            {
//                                WriteLog(pcbplc_logger, (char *) json_object_to_json_string(json_config), LOG_INFO);
//                            }
//
//                            json_object_put(json_config);
//                        }
//                    }
//                    else
//                    {
//                        WriteLog(pcbplc_logger, "configuration file not found", LOG_CRITICAL);
//                    }
//                }
//                else if (strcmp(request, "get_recipe_config") == 0)
//                {
//                    char filename[33] = RECIPE_JSON_FILE_PATH;
//                    if(filename)
//                    {
//                        json_object *json_config = json_object_from_file(filename);
//                        if(json_config)
//                        {
//                            message_bus_send(message_bus, node_name, Response, Text,
//                                             (char *) json_object_to_json_string(json_config), strlen(json_object_to_json_string(json_config)));
//
//                            if (gpcbplcCnfg.debug)
//                            {
//                                WriteLog(pcbplc_logger, (char *) json_object_to_json_string(json_config), LOG_INFO);
//                            }
//
//                            json_object_put(json_config);
//                        }
//                    }
//                    else
//                    {
//                        WriteLog(pcbplc_logger, "configuration file not found", LOG_CRITICAL);
//                    }
//                }
//                if (strcmp(request, "set_recipe_config") == 0)
//                {
//                    char filename[33] = RECIPE_JSON_FILE_PATH;
//                    if(filename)
//                    {
//                        int d = open(filename,O_WRONLY|O_CREAT|O_TRUNC, 0600);
//                        if(d > -1)
//                        {
//                            json_object_to_fd(d, obj_request_para, JSON_C_TO_STRING_PRETTY| JSON_C_TO_STRING_NOSLASHESCAPE);
//                            close(d);
//                        }
//                    }
//                    gPcbplcInfo.mReloadReceipe = true;
//
//                    network_reply(node_name);
//                }
//                if(requestdata)
//                {
//                    free(requestdata);
//                    requestdata = NULL;
//                }
//
//                if(root_rt)
//                {
//                    json_object_put(root_rt);
//                    root_rt = NULL;
//                }
//            }
//            break;
//        }
//        case Response: {
//            printf("Response Recived: %s\n", messagebuffer);
//            break;
//        }
//        default:
//        {
//            printf("Misc Recived: %s\n", messagebuffer);
//            break;
//        }
//    }
//}

//static void on_disconnect_event(void)
//{
//    if(message_bus != NULL)
//    {
//        message_bus_close(message_bus);
//        message_bus_release(message_bus);
//    }
//
//    message_bus = NULL;
//    int retryCount = 1;
//    while(retryCount <= 3)
//    {
//        retryCount++;
//        message_bus = message_bus_initialize(on_network_event, on_disconnect_event);
//        if (!message_bus)
//        {
//            sleep(10);
//            WriteLog(pcbplc_logger, "Could not initialize IPC", LOG_ERROR);
//            continue;
//        }
//
//        if (!message_bus_open(message_bus))
//        {
//            sleep(10);
//            WriteLog(pcbplc_logger, "Could not open IPC", LOG_ERROR);
//            continue;
//        }
//        break;
//    }
//}

//void on_signal_received(SignalType stype) {
//    switch (stype) {
//        case Suspend: {
//            WriteLog(pcbplc_logger, "SUSPEND SIGNAL", LOG_CRITICAL);
//            break;
//        }
//        case Resume: {
//            WriteLog(pcbplc_logger, "RESUME SIGNAL", LOG_CRITICAL);
//            continue_loop = false;
//            sleep(1);
//            exit(EXIT_FAILURE);
//        }
//        case Shutdown: {
//            WriteLog(pcbplc_logger, "SHUTDOWN SIGNAL", LOG_CRITICAL);
//            continue_loop = false;
//            sleep(2);
//            exit(EXIT_SUCCESS);
//        }
//        case Alarm: {
//            WriteLog(pcbplc_logger, "ALARM SIGNAL", LOG_CRITICAL);
//            continue_loop = false;
//            sleep(2);
//            exit(EXIT_FAILURE);
//        }
//        case Reset: {
//            WriteLog(pcbplc_logger, "RESET SIGNAL", LOG_CRITICAL);
//            break;
//        }
//        case ChildExit: {
//            WriteLog(pcbplc_logger, "CHILD PROCESS EXIT SIGNAL", LOG_CRITICAL);
//            break;
//        }
//        case Userdefined1: {
//            WriteLog(pcbplc_logger, "USER DEFINED 1 SIGNAL", LOG_CRITICAL);
//            break;
//        }
//        case Userdefined2: {
//            WriteLog(pcbplc_logger, "USER DEFINED 2 SIGNAL", LOG_CRITICAL);
//            break;
//        }
//        case WindowResized: {
//            WriteLog(pcbplc_logger, "WINDOW RESIZED SIGNAL", LOG_CRITICAL);
//            break;
//        }
//        default: {
//            WriteLog(pcbplc_logger, "UNKNOWN SIGNAL", LOG_CRITICAL);
//            break;
//        }
//    }
//}

//static void prepare_and_send_tag_payload()
//{
//    json_object * jobj_root = json_object_new_object();
//    json_object * jobj_Para_array = json_object_new_array();
//
//    json_object_object_add(jobj_root, "request" , json_object_new_string("add_device"));
//    json_object_object_add(jobj_root, "request_parameters" , jobj_Para_array);
//    json_object * jobj_device_ = json_object_new_object();
//    json_object_object_add(jobj_device_, "devicename" , json_object_new_string("PCBPLC"));
//    json_object_object_add(jobj_device_, "protocol" , json_object_new_string("PCBPLC"));
//
//    json_object_object_add(jobj_root, "device_parameters" , jobj_device_);
//
//    if(message_bus != NULL)
//    {
//        if (message_bus_send(message_bus, "service_tag", Request, Text, (char *) json_object_to_json_string(jobj_root),
//                             (int32_t) strlen(json_object_to_json_string(jobj_root))))
//        {
//        }
//        else
//        {
//            WriteLog(pcbplc_logger, "Send failure over IPC", LOG_ERROR);
//        }
//    }
//    json_object_put(jobj_root);
//    return;
//}

//static void Store_General_purpose_analog_parameter(void)
//{
//    char tBuff[32] = {0,};
//    uint32_t index = 161;
//
//    json_object *obj_root = json_object_new_object();
//    if(obj_root)
//    {
//        for (int idx = 0 ; idx < MAX_GEN_ANA_PARA ; ++idx)
//        {
//            sprintf(tBuff, "%d", index + idx);
//            json_object_object_add(obj_root, tBuff, json_object_new_double(gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG + idx] ));
//        }
//
//        #if 0
//        int d = open("debug.json",O_WRONLY|O_CREAT|O_TRUNC, 0600);
//        if(d > -1)
//        {
//            int rv = json_object_to_fd(d, obj_root, JSON_C_TO_STRING_PRETTY| JSON_C_TO_STRING_NOSLASHESCAPE);
//            close(d);
//        }
//        #endif
//
//        FILE *fp = fopen(ANALOG_JSON_FILE_PATH , "w+");
//        fputs( json_object_to_json_string(obj_root), fp);
//        fclose(fp);
//
//        json_object_put(obj_root);
//        obj_root = NULL;
//    }
//}
//
//static void Restore_General_purpose_analog_parameter(void)
//{
//    char tBuff[32] = {0,};
//    uint32_t index = 161;
//
//
//    json_object *parse_result = json_object_from_file(ANALOG_JSON_FILE_PATH);
//    if(parse_result != NULL)
//    {
//        for (int idx = 0 ; idx < MAX_GEN_ANA_PARA ; ++idx)
//        {
//            sprintf(tBuff, "%d", index + idx);
//            json_object *obj_analog = json_object_object_get(parse_result,tBuff);
//            if(obj_analog != NULL)
//            {
//                gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG + idx] = json_object_get_double(obj_analog);
//                signal_arr[idx] = gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG + idx];
//            }
//        }
//
//        json_object_put(parse_result);
//        parse_result = NULL;
//    }
//    else
//    {
//        WriteLog(pcbplc_logger, "Failed to Get debug.json at reboot", LOG_ERROR);
//    }
//}
//
//static void Log_General_purpose_analog_parameter(void)
//{
//    char tBuff[32] = {0,};
//    uint32_t index = 161;
//
//    if(gpcbplcCnfg.debug)
//    {
//        WriteLog(pcbplc_logger, "***************** General Analog parameters *********************", LOG_INFO);
//        for (int idx = 0; idx < MAX_GEN_ANA_PARA; ++idx)
//        {
//            sprintf(tBuff, "%d: %3.2f", index + idx, signal_arr[idx]);
//            WriteLog(pcbplc_logger, tBuff, LOG_INFO);
//        }
//        WriteLog(pcbplc_logger, "***************** End of General Analog parameters *****************", LOG_INFO);
//    }
//}

//static void network_reply(const char * topic_name)
//{
//    struct timeval tv;
//    gettimeofday(&tv, NULL);
//
//    uint64_t millisecondsSinceEpoch =
//            (uint64_t) (tv.tv_sec) * 1000 +
//            (uint64_t) (tv.tv_usec) / 1000;
//
//    json_object * jobj_reply   = json_object_new_object();
//    json_object_object_add(jobj_reply, "timestamp" ,
//                           json_object_new_int64(millisecondsSinceEpoch));
//    json_object_object_add(jobj_reply, "status", json_object_new_string("success"));
//    json_object_object_add(jobj_reply, "response_code", json_object_new_int(0));
//    json_object_object_add(jobj_reply, "response_message", json_object_new_string("success"));
//
//    message_bus_send(message_bus, topic_name, Response, Text,
//                     (char *) json_object_to_json_string(jobj_reply),strlen(json_object_to_json_string(jobj_reply)) );
//    json_object_put(jobj_reply);
//}

int update_do_status_key(int pin_no, int pin_value)
{
//    if( (gpcbplcCnfg.mMaxDoEnabled >= pin_no) && (pin_no != 0))
//    {
//        configuration_create_backup(NULL);
//
//        char *filename = (char *) configuration_get_filepath();
//        if (filename)
//        {
//            json_object *parse_result = json_object_from_file(filename);
//            if (parse_result)
//            {
//                json_object *do_Status = json_object_object_get(parse_result, "do_status");
//                for (int i = 0; i < pin_no; i++)
//                {
//                    if ((pin_no - 1) == i)
//                    {
//                        json_object_array_put_idx(do_Status, i, json_object_new_int(pin_value));
//                        break;
//                    }
//                }
//
//                int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                if (d > -1)
//                {
//                    json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                    close(d);
//                }
//
//                free(filename);
//                filename = NULL;
//                json_object_put(parse_result);
//                parse_result = NULL;
//                return 0;
//            }
//        }
//        else
//        {
//            WriteLog(pcbplc_logger, "configuration file not found", LOG_CRITICAL);
//            return -1;
//        }
//    }
//    else
//    {
//        WriteLog(pcbplc_logger, "Invalid pin number", LOG_ERROR);
        return -1;
//    }
}

unsigned char checkFileAvibility(unsigned char file)
{
	unsigned char result=0;
	//Todo : Add PLC and REC File Validation logic Maulin
	//return 0 on validate 1 on fail

	if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		if(file == 1)
		{
			if(gPlcRecFlash.mPlcFileLength)
			{
				result = varify_OTA_CRC(gPlcRecFlash.mPlcFileCRC,gPlcRecFlash.mPlcFileLength,PUB_FILE_START_ADDRESS);
			}
			else
			{
				result = 1;
			}

		}
		else if(file == 2)
		{
			if(gPlcRecFlash.mRecFileLength)
			{
				result = varify_OTA_CRC(gPlcRecFlash.mRecFileCRC,gPlcRecFlash.mRecFileLength,REC_FILE_START_ADDRESS);
			}
			else
			{
				result = 1;
			}
		}
		xSemaphoreGive(sendExternalFlashSemaphore);
	}
	else
	{
		result =1;
	}



	return result;
}
