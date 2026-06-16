#ifndef SAMPLEFUNCTION_H
#define SAMPLEFUNCTION_H

#include <vector>

namespace tim
{

class SampleFunction
{
public:
    SampleFunction();
    SampleFunction(const std::vector<float>&);
    SampleFunction(std::initializer_list<float>);

    SampleFunction(const SampleFunction&) = default;
    SampleFunction& operator=(const SampleFunction&) = default;

    SampleFunction& addSample(float);
    void clear();

    float operator()(float) const;

	static SampleFunction interpolate(const SampleFunction& f1, const SampleFunction& f2, float coef);

private:
    std::vector<float> _samples;
};

}

#endif // SAMPLEFUNCTION_H
