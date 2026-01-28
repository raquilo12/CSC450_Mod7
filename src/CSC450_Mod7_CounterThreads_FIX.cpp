//============================================================================
// Name        : CSC450_Mod7_CounterThreads_FIX.cpp
// Author      : raquilo2
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

struct SharedState {
    int counterValue = 0;
    bool reachedTwenty = false;
    std::mutex m;
    std::condition_variable cv;
};

static void safePrint(std::mutex& coutMutex, const std::string& label, int value) {
    // Prevent interleaved output when multiple threads write to the console.
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << label << value << std::endl;
}

int main() {
    SharedState state;

    // Thread 1: count up to 20, then signal Thread 2.
    std::thread tUp([&state]() {
        for (int i = 0; i <= 20; ++i) {
            {
                std::lock_guard<std::mutex> lock(state.m);
                state.counterValue = i;
                if (i == 20) {
                    state.reachedTwenty = true;
                }
            }

            safePrint(state.m, "UP  : ", i);

            if (i == 20) {
                state.cv.notify_one();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // Thread 2: wait until Thread 1 reaches 20, then count down to 0.
    std::thread tDown([&state]() {
        {
            std::unique_lock<std::mutex> lock(state.m);
            state.cv.wait(lock, [&state]() { return state.reachedTwenty; });
        }

        for (int i = 20; i >= 0; --i) {
            {
                std::lock_guard<std::mutex> lock(state.m);
                state.counterValue = i;
            }

            safePrint(state.m, "DOWN: ", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    tUp.join();
    tDown.join();

    std::cout << "Done." << std::endl;
    return 0;
}
