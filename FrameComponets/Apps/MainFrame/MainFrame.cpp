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
#include "GetCoord.hpp"
#include "M0Commander.hpp"
#include "System.hpp"
#include "VisionReceiver.hpp"
#include "VisionTrack.hpp"
#include "std_cpp.h"
#include "stm32f4xx_hal.h"

/**
 * @brief 主程序入口
 * @warning 严禁阻塞
 */
void MainFrameCpp() {
    VisionReceiver::GetInstance().Init(&huart1); // 初始化视觉接收器，绑定 UART1 句柄
    M0Commander::GetInstance().Init(&huart3);    // 初始化 M0Commander，绑定 UART3 句柄

    // 注册应用
    System.RegistApp(get_coord_app); // 蓝牙坐标接收
    System.RegistApp(vision_track);  // 视觉巡线

    // 临时：强制启动系统（后面由蓝牙 START 控制）
    System.system_started = true;
}
