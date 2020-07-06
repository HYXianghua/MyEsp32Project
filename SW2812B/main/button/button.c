#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_types.h"
#include "driver/gpio.h"

static struct button *Head_Button = NULL;

/*******************************************************************
 *                         函数声明
 *******************************************************************/
static char *StrnCopy(char *dst, const char *src, uint32_t n);
static void Add_Button(Button_t *btn);

static uint8_t Read_Button_Level(uint8_t pin) //按键当前电平
{
  return gpio_get_level(pin);
}
/************************************************************
 * @brief   按键创建
 * @param   name : 按键名称
 * @param   btn : 按键结构体
 * @param   read_btn_level :
 *按键电平读取函数，需要用户自己实现返回uint8_t类型的电平
 * @param   btn_trigger_level : 按键触发电平
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 * @note    NULL
 ***********************************************************/
void Button_Create(const char *name, Button_t *btn, uint8_t pin,
                   uint8_t btn_trigger_level)
{
  memset((char *)btn, 0,
         sizeof(struct button)); //清除结构体信息，建议用户在之前清除

  StrnCopy(btn->Name, name, BTN_NAME_MAX); /* 创建按键名称 */

  btn->Button_State = NONE_TRIGGER;         //按键状态
  btn->Button_Last_State = NONE_TRIGGER;    //按键上一次状态
  btn->Button_Trigger_Event = NONE_TRIGGER; //按键触发事件
  // btn->Read_Button_Level = read_btn_level;           //按键读电平函数
  btn->Pin = pin;
  btn->Button_Trigger_Level = btn_trigger_level;        //按键触发电平
  btn->Button_Last_Level = Read_Button_Level(btn->Pin); //按键当前电平
  // btn->Button_Last_Level = btn->Read_Button_Level(); //按键当前电平
  btn->Debounce_Time = 0;
  btn->Next = NULL;

  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = 1ULL << pin;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 0;
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);

  Add_Button(btn); //创建的时候添加到单链表中
}

/************************************************************
 * @brief   按键触发事件与回调函数映射链接起来
 * @param   btn : 按键结构体
 * @param   btn_event : 按键触发事件
 * @param   btn_callback : 按键触发之后的回调处理函数。需要用户实现
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 ***********************************************************/
void Button_Attach(Button_t *btn, Button_Event btn_event,
                   Button_CallBack btn_callback)
{
  if (btn == NULL)
  {
  }

  if (BUTTON_ALL_RIGGER == btn_event)
  {
    uint8_t i;
    for (i = 0; i < number_of_event - 1; i++)
      btn->CallBack_Function[i] =
          btn_callback; //按键事件触发的回调函数，用于处理按键事件
  }
  else
  {
    btn->CallBack_Function[btn_event] =
        btn_callback; //按键事件触发的回调函数，用于处理按键事件
  }
}

/************************************************************
 * @brief   删除一个已经创建的按键
 * @param   NULL
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 * @note    NULL
 ***********************************************************/
void Button_Delete(Button_t *btn)
{
  struct button **curr;
  for (curr = &Head_Button; *curr;)
  {
    struct button *entry = *curr;
    if (entry == btn)
    {
      *curr = entry->Next;
    }
    else
    {
      curr = &entry->Next;
    }
  }
}

/************************************************************
 * @brief   获取按键触发的事件
 * @param   NULL
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 ***********************************************************/
void Get_Button_EventInfo(Button_t *btn)
{
  //按键事件触发的回调函数，用于处理按键事件
  uint8_t i;
  for (i = 0; i < number_of_event - 1; i++)
  {
    if (btn->CallBack_Function[i] != 0)
    {
      // PRINT_INFO("Button_Event:%d",i);
    }
  }
}

uint8_t Get_Button_Event(Button_t *btn)
{
  return (uint8_t)(btn->Button_Trigger_Event);
}

/************************************************************
 * @brief   获取按键触发的事件
 * @param   NULL
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 ***********************************************************/
uint8_t Get_Button_State(Button_t *btn) { return (uint8_t)(btn->Button_State); }

/************************************************************
 * @brief   按键周期处理函数
 * @param   btn:处理的按键
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 * @note    必须以一定周期调用此函数，建议周期为20~50ms
 ***********************************************************/
void Button_Cycle_Process(Button_t *btn)
{
  uint8_t current_level = Read_Button_Level(btn->Pin); //获取当前按键电平

  if ((current_level != btn->Button_Last_Level) &&
      (++(btn->Debounce_Time) >= BUTTON_DEBOUNCE_TIME)) //按键电平发生变化，消抖
  {
    btn->Button_Last_Level = current_level; //更新当前按键电平
    btn->Debounce_Time = 0;                 //确定了是按下

    //如果按键是没被按下的，改变按键状态为按下(首次按下/双击按下)
    if (btn->Button_State == NONE_TRIGGER)
    {
      btn->Button_State = BUTTON_DOWM;
    }
    //释放按键
    else if (btn->Button_State == BUTTON_DOWM)
    {
      btn->Button_State = BUTTON_UP;
      // PRINT_DEBUG("释放了按键");
    }
  }

  switch (btn->Button_State)
  {
  case BUTTON_DOWM: // 按下状态
  {
    if (btn->Button_Last_Level == btn->Button_Trigger_Level) //按键按下
    {
      if (btn->Button_Trigger_Event != BUTTON_LONG)
      {
        btn->Button_Trigger_Event = BUTTON_DOWM;
      }

      if (++(btn->Long_Time) >= BUTTON_LONG_TIME) //释放按键前更新触发事件为长按
      {
        if (++(btn->Button_Cycle) >= BUTTON_LONG_CYCLE) //连续触发长按的周期
        {
          btn->Button_Cycle = 0;
          btn->Button_Trigger_Event = BUTTON_LONG;
          TRIGGER_CB(BUTTON_LONG); //长按
        }
        if (btn->Long_Time == 0xFF) //更新时间溢出
        {
          btn->Long_Time = BUTTON_LONG_TIME;
        }
      }
    }
  }
  break;
  case BUTTON_UP: // 弹起状态
  {
    if (btn->Button_Trigger_Event == BUTTON_DOWM) //触发单击
    {
      btn->Timer_Count = 0;
      btn->Long_Time = 0;      //检测长按失败，清0
      TRIGGER_CB(BUTTON_DOWM); //单击
      btn->Button_State = NONE_TRIGGER;
      btn->Button_Last_State = BUTTON_DOWM;
    }
    else if (btn->Button_Trigger_Event == BUTTON_LONG)
    {
      TRIGGER_CB(BUTTON_LONG_FREE); //长按释放
      btn->Long_Time = 0;
      btn->Button_State = NONE_TRIGGER;
      btn->Button_Last_State = BUTTON_LONG;
      btn->Button_Trigger_Event = BUTTON_DOWM;
    }
  }
  break;
  default:
    break;
  }
}

/************************************************************
 * @brief   遍历的方式扫描按键，不会丢失每个按键
 * @param   NULL
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 * @note    此函数要周期调用，建议20-50ms调用一次
 ***********************************************************/
static void Button_Process(void *arg)
{
  struct button *pass_btn = NULL;
  while (1)
  {
    vTaskDelay(50 / portTICK_RATE_MS);
    for (pass_btn = Head_Button; pass_btn != NULL; pass_btn = pass_btn->Next)
    {
      Button_Cycle_Process(pass_btn);
    }
  }
}

/**************************** 以下是内部调用函数 ********************/

/************************************************************
 * @brief   拷贝指定长度字符串
 * @param   NULL
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 * @note    NULL
 ***********************************************************/
static char *StrnCopy(char *dst, const char *src, uint32_t n)
{
  if (n != 0)
  {
    char *d = dst;
    const char *s = src;
    do
    {
      if ((*d++ = *s++) == 0)
      {
        while (--n != 0)
          *d++ = 0;
        break;
      }
    } while (--n != 0);
  }
  return (dst);
}

/************************************************************
 * @brief   使用单链表将按键连接起来
 * @param   NULL
 * @return  NULL
 * @author  jiejie
 * @github  https://github.com/jiejieTop
 * @date    2018-xx-xx
 * @version v1.0
 * @note    NULL
 ***********************************************************/
static void Add_Button(Button_t *btn)
{
  struct button *pass_btn = Head_Button;

  while (pass_btn != NULL)
  {
    pass_btn = pass_btn->Next;
  }

  btn->Next = Head_Button;
  Head_Button = btn;
}

void but_init(void)
{
  xTaskCreate(Button_Process, "button_task", 2048, NULL, 10, NULL);
}