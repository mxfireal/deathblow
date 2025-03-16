// mxfireal - i've moved all the stuff related to rendering the scene here
// since i know everything will be using glsl, it can be done in one pass

enum
{
    kQSRSCProgramWorld,
    kQSRSCProgramWarp, //water
    kQSRSCProgramSky,
    kQSRSCProgram_Count,
};

extern int qsrc_programs[];

void QSRSC_RenderScene();