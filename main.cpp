#include <stdio.h>
#include "PolyDetector.h"

int main(int argc, char **argv)
{
    std::vector<PolyLine> lines = {
            {{-15, 0},   {15, 0}     }  ,
            {{-15, -15}, {15, -15}  }  ,
            {{-10, -20}, {-10, 20} }  , 
            {{10, -20}, {10, -5}   }  ,
            {{5, -10},  {15, -10}    }  , 
            {{8, -12},   {8, 20}   }  ,
    };

    PolyDetector pd;
	//pd.verbose = 1;
	
    for (auto &l : lines)
        pd.AddLine(l);

    if (!pd.DetectPolygons())
    {
        logoutf("%s", "WARN: cannot detect polys!");
        return -1;
    }
	
	logoutf("nPolys:%u dissolveSteps:%u lines:%u", uint32_t(pd.polys.size()), pd.dissolveCount + 1, uint32_t(pd.lines.size()));

#if 0
    for (auto &poly : pd.polys)
    {
        for (auto &p : poly.p)
        {
            logoutf("[%u] p:{%f %f}", poly.id, p.x, p.y);
        }
    }
#endif

    return 0;
}

