#include<thread>
#include<atomic>
///true = continue execution, false = pause thread
std::atomic<int> flag;
std::thread t;

#ifdef __cplusplus
extern "C" {
#endif
    void start_beep(int stopFlag){
        flag = stopFlag;
        while(flag) {

            //TODO: Generate sound while the flag is true
            //Edit: async behavior can be achieved solely with SDL Library, which is anyway in use for the GUI

        }
    }

    void update_flag(int stopFlag) {
        flag = stopFlag;
    }

#ifdef __cplusplus
}
#endif
