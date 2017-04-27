//knn libraries
#include "fftReal.hpp"
#include <vector>
#include <math.h>
#include <set>
#include <map>
 
class Individual {
private:
    std::string label;
    float s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40;
   
public:
    Individual(float s1, float s2, float s3, float s4, float s5, float s6, float s7, float s8, float s9, float s10,
    float s11, float s12, float s13, float s14, float s15, float s16, float s17, float s18, float s19, float s20,
    float s21, float s22, float s23, float s24, float s25, float s26, float s27, float s28, float s29, float s30,
    float s31, float s32, float s33, float s34, float s35, float s36, float s37, float s38, float s39, float s40, std::string label);
   
    std::string getLabel();
 
    float getS1();
 
    float getS2();
   
    float getS3();
   
    float getS4();
   
    float getS5();
   
    float getS6();
   
    float getS7();
 
    float getS8();
   
    float getS9();
   
    float getS10();
   
    float getS11();
 
    float getS12();
   
    float getS13();
   
    float getS14();
   
    float getS15();
   
    float getS16();
   
    float getS17();
 
    float getS18();
   
    float getS19();
   
    float getS20();
   
    float getS21();
 
    float getS22();
   
    float getS23();
   
    float getS24();
   
    float getS25();
   
    float getS26();
   
    float getS27();
 
    float getS28();
   
    float getS29();
   
    float getS30();
   
    float getS31();
 
    float getS32();
   
    float getS33();
   
    float getS34();
   
    float getS35();
   
    float getS36();
   
    float getS37();
 
    float getS38();
   
    float getS39();
   
    float getS40();
   
};
 
class Machine_Learning {
 
private:
    AnalogIn mic_in;
public:
    Machine_Learning(PinName input_mic);
 
    void removeBias(float *array);
 
    void preFFT(float array[], float *arrayBin, int ind);
 
    void putBack(float *array, Complex arrayBin[], int ind);
   
    double getEuclideanDistance(Individual ind1, Individual ind2);
 
    std::string classifySample(std::vector<Individual>& individual, Individual new_Example, int K);
 
    string readSample();
};