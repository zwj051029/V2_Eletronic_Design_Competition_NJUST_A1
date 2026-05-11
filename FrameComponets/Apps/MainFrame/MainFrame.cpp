////////////////////////////////////////////////////////////////////
//                          _ooOoo_                               //
//                         o8888888o                              //
//                         88" . "88                              //
//                         (| ^_^ |)                              //
//                         O\  =  /O                              //
//                      ____/`---'\____                           //
//                    .'  \\|     |//  `.                         //
//                   /  \\|||  :  |||//  \                        //
//                  /  _||||| -:- |||||-  \                       //
//                  |   | \\\  -  /// |   |                       //
//                  | \_|  ''\---/''  |   |                       //
//                  \  .-\__  `-`  ___/-. /                       //
//                ___`. .'  /--.--\  `. . ___                     //
//              ."" '<  `.___\_<|>_/___.'  >'"".                  //
//            | | :  `- \`.;`\ _ /`;.`/ - ` : | |                 //
//            \  \ `-.   \_ __\ /__ _/   .-` /  /                 //
//      ========`-.____`-.___\_____/___.-`____.-'========         //
//                           `=---='                              //
//      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^        //
//          佛祖保佑          永不宕机          永无BUG            //
////////////////////////////////////////////////////////////////////
#include "MainFrame.hpp"
#include "bsp_uart.h"
#include "std_cpp.h"
#include "stm32f4xx_hal.h"

BspUart_Instance uart1_instance;
void TestRxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len);

/**
 * @brief 主程序入口
 * @warning 严禁阻塞
 */
void MainFrameCpp() {

    // 注册 UART1 实例，使用中断模式接收，DMA 模式发送，接收缓冲区大小为 128 字节
    BspUart_Register(&uart1_instance, &huart1, BSP_UART_MODE_DMA, BSP_UART_MODE_DMA, 128, TestRxCallback);

    while (1) {
    }
}

void TestRxCallback(BspUart_Instance *inst, uint8_t *data, uint16_t len) {
    // 收到数据后原样返回
    BspUart_Transmit(inst, data, len);
}
