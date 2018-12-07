#include "core/timer/timer.h"

Timer timer;

void onTimer()
{
    auto id = timer.Add(16, std::bind(&onTimer));
    timer.Del(id);
    std::cout << "onTimer" << std::endl;
}

int main()
{
    timer.Add(16, std::bind(&onTimer));
    while (true)
    {
        timer.Update(std::chrono::high_resolution_clock::now());
    }
    return 0;
}