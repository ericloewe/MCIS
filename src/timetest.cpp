
#include <iostream>
#include <chrono>


int main()
{
    auto nextTick = std::chrono::high_resolution_clock::now();
    //std::chrono::high_resolution_clock::duration oneSecond(std::chrono::duration<long long>(1));
    auto sampleTime = std::chrono::microseconds( (int)(10e6 / 120));
    //auto sampleTime = std::chrono::microseconds((unsigned long)(10e9));

    std::cout << "std::chrono::duration = " << sampleTime.count() << " us" << std::endl;
    std::cout << "                      = " << sampleTime.count() * 1e-6 << " s" << std::endl;
    std::cout << "sampleRate = " << 120 << std::endl;
    std::cout << "Conversion: (int)(1e6 / 120) = " << (int)(1e6 / 120) << std::endl;
    std::cout << "Conversion: (1e6 / 120) = " << (1e6 / 120) << std::endl;
    std::cout << "Conversion: 1e6 = " << 1e6 << std::endl;

    for (int i = 0; i < 120; i++)
    //while (true)
    {
        nextTick += sampleTime;
    }
}

