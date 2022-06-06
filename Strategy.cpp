#include <cmath>
#include "consts.h"
#include <d2d1.h>

class IFunctions
{
public:
    virtual ~IFunctions() {}
    virtual void RunFunction(int cx, int cy, D2D1_POINT_2F* apt) = 0;
};

class Sin : public IFunctions
{
public:
    void RunFunction(int cx, int cy, D2D1_POINT_2F* apt) override
    {
        for (int i = 0; i < NUM; i++)
        {
            apt[i].x = i * cx / NUM;
            apt[i].y = static_cast<int>(cy / 2 * (1 - sin(TWOPI * i / NUM)));
        }
    }
};

class Sqrt final : public IFunctions
{
public:
    void RunFunction(const int cx, const int cy, D2D1_POINT_2F* apt) override
    {
        for (int i = 0; i < NUM; i++)
        {
            apt[i].x = cx/2 + cx * i / (2 * NUM);
            apt[i].y = static_cast<int>(cy / 2 - sqrt(i) * cy / (2 * sqrt(NUM)));
        }
    }
};

class para final : public IFunctions
{
public:
    void RunFunction(const int cx, const int cy, D2D1_POINT_2F* apt) override
    {
	    const double coeff = 2.0 * cy / cx;
        for (int i = 0; i < NUM; i++)
        {
	        const double x = cx * i / NUM;
            apt[i].x = x;
            apt[i].y = static_cast<int>(coeff * (-x * x / cx + x));
        }
    }
};

class Hyper : public IFunctions
{
public:
    void RunFunction(const int cx, const int cy, D2D1_POINT_2F* apt) override
    {
        double y;
        const double coeff = cx * cy / 64.0;
        for (int i = 0; i < NUM; i++)
        {
            double x = i * static_cast<double>(cx) / NUM;
            if (i < NUM / 2 || i > NUM / 2) {
                y = static_cast<int>(-coeff / (x - cx / 2) + cy / 2);
            }
            else {
                y = cy / 2;
            }
            if (i < NUM / 2) {
                apt[i].x = x;
                apt[i].y = y;
            }
            else if (i > NUM / 2) {
                apt[i].x = x;
                apt[i].y = y;
            }
        }
    }
};

class function {
    public:
        function(IFunctions *func): p_(func) { }
        ~function() { delete p_; }
        void set_func(IFunctions* func) { p_ = func; }
        void run_function(const int cx, const int cy, D2D1_POINT_2F* apt) const
        {
            return p_->RunFunction(cx, cy, apt);
        }
    private:
        IFunctions* p_;

};