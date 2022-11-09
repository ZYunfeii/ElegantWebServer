#include "threadpoolv2.h"
#include <iostream>
#include <thread>
#include <time.h>
#include <chrono>

using namespace std;

void task1() {
    cout << "task1 done!" << endl;
}

void task2() {
    cout << "task2 done!" << endl;
}

void task3() {
    cout << "task3 done!" << endl;
}

void task4() {
    cout << "task4 done!" << endl;
}

int main(int argc, char** argv) {
    auto p = new ThreadPoolV2(4);  
    thread([p](){
        for(int i = 0; i < 5; ++i) {
            p->addTask(bind(task1));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();
    thread([p](){
        for(int i = 0; i < 5; ++i) {
            p->addTask(bind(task2));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();
    thread([p](){
        for(int i = 0; i < 5; ++i) {
            p->addTask(bind(task3));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();
    thread([p](){
        for(int i = 0; i < 5; ++i) {
            p->addTask(bind(task4));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}