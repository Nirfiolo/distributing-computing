#include "tbb\tbb.h"

#include <stdio.h>
#include <memory>
#include <windows.h>


constexpr int size = 5000;
int world_size = 2;

std::unique_ptr<float[]> a;
std::unique_ptr<float[]> b;
std::unique_ptr<float[]> c;


void init_matrix(std::unique_ptr<float[]> & a, bool is_set_to_zero = false)
{
    a = std::make_unique<float[]>(size * size);

    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            a[i * size + j] = 0.f;
            if (!is_set_to_zero)
            {
                a[i * size + j] = (static_cast<float>(rand() % 10000) / 10000.f - 0.5f) * 50.f;
            }
        }
    }
}

float multiplication_time(void (*multiplier)())
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER t1;
    LARGE_INTEGER t2;

    QueryPerformanceFrequency(&frequency);

    QueryPerformanceCounter(&t1);

    multiplier();

    QueryPerformanceCounter(&t2);

    return static_cast<float>(t2.QuadPart - t1.QuadPart) / static_cast<float>(frequency.QuadPart);
}

void consistent()
{
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            float temp = 0.f;
            for (size_t k = 0; k < size; ++k)
            {
                temp += a[i * size + k] * b[k * size + j];
            }
            c[i * size + j] = temp;
        }
    }
}

void tape_circuit()
{
    tbb::task_scheduler_init init(world_size);

    tbb::parallel_for(tbb::blocked_range<size_t>(0, size),
        [&](tbb::blocked_range<size_t> r)
        {
            for (size_t i = r.begin(); i < r.end(); ++i)
            {
                for (size_t j = 0; j < size; ++j)
                {
                    float temp = 0.f;
                    for (size_t k = 0; k < size; ++k)
                    {
                        temp += a[i * size + k] * b[k * size + j];
                    }
                    c[i * size + j] = temp;
                }
            }
        });
}


int main(int argc, char * argv[])
{
    init_matrix(a);
    init_matrix(b);
    init_matrix(c, true);

    printf("%f\n", multiplication_time(consistent));

    world_size = 2;
    printf("%f\n", multiplication_time(tape_circuit));
    world_size = 4;
    printf("%f\n", multiplication_time(tape_circuit));
}