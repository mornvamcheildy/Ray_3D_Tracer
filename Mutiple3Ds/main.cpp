
#include <iostream>

class One
{

public:
    double drova[9];
    One(double fisrt) : drova{fisrt, 16.0, 18.0, 19.0, 22.0, 24.0, 29.0, 30.0, 34.0} {};
};

inline double operator/(double callee, const One &d)
{
    auto drez = d.drova[0] + d.drova[1] + d.drova[2] + d.drova[3] + d.drova[4] + d.drova[5] + d.drova[6] + d.drova[7] + d.drova[8];
    double mean = drez / callee;
    double median = d.drova[4];

    std::cout << mean << std::endl;
    std::cout << median << std::endl;
    
    //std::cout << callee << std::endl;
    //std::cout << callee << std::endl;

    return mean;
};

// inline One unit_vector(One v) { return (1.0 / v.drova) * v; }

int main()
{
    One onz(15.0);
    double b = 9 / onz;

    //std::cout << b << std::endl;

    return 0;
}