#include <slvs.h>

int main() {
    const Slvs_Param param = Slvs_MakeParam(1, 1, 0.0);

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    Slvs_QuaternionU(1.0, 0.0, 0.0, 0.0, &x, &y, &z);

    return param.h == 1 && x == 1.0 && y == 0.0 && z == 0.0 ? 0 : 1;
}
