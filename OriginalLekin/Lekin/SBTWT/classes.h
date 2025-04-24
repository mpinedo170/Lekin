#ifndef CLASSES_H
#define CLASSES_H

class Input_File_Jobs
{
    int jobs;
    int* w;
    int* r;
    double* f;
    int* N_opers;
    int* d;
    int** workcenters;
    int** p;

public:
    void put_jobs(int j)
    {
        jobs = j;
    }
    int get_jobs()
    {
        return (jobs);
    }

    void tam_w(int tam)
    {
        w = new int[tam];
    }
    void put_w(int v, int i)
    {
        w[i] = v;
    }
    int* get_w()
    {
        return (w);
    }
    int get_w1(int i)
    {
        return (w[i]);
    }

    void tam_r(int tai)
    {
        r = new int[tam];
    }
    void put_r(int v, int i)
    {
        r[i] = v;
    }
    int* get_r()
    {
        return (r);
    }
    int get_r1(int i)
    {
        return (r[i]);
    }
    int** get_r2(){return

        void tam_f(int tam){f = new double[tam];
} void put_f(double v, int i)
{
    f[i] = v;
}
double* get_f()
{
    return (f);
}
double get_f1(int i)
{
    return (f[i]);
}

void tam_N_opers(int tam)
{
    N_opers = new int[tam];
}
void put_N_opers(int v, int i)
{
    N_opers[i] = v;
}
int* get_N_opers()
{
    return (N_opers);
}
int get_N_opers1(int i)
{
    return (N_opers[i]);
}

void tam_d(int tam)
{
    d = new int[tam];
}
void put_d(int v, int i)
{
    d[i] = v;
}
int* get_d()
{
    return (d);
}
int get_d1(int i)
{
    return (d[i]);
}

void tam_workcenters1(int tam)
{
    workcenters = new int*[tam];
}
void tam_workcenters2(int tam, int i)
{
    workcenters[i] = new int[tam];
}
void put_workcenters(int v, int i, int j)
{
    workcenters[i][j] = v;
}
int** get_workcenters()
{
    return (workcenters);
}
int get_workcenters1(int i, int j)
{
    return (workcenters[i][j]);
}

void tam_p1(int tam)
{
    p = new int*[tam];
}
void tam_p2(int tam, int i)
{
    p[i] = new int[tam];
}
void put_p(int v, int i, int j)
{
    p[i][j] = v;
}
int** get_p()
{
    return (p);
}
int get_p1(int i, int j)
{
    return (p[i][j]);
}
}
;

class Input_File_Workcenters
{
    int N_workcenter;
    int* N_maq;
    int** list_speeds;

public:
    void put_N_workcenter(int n)
    {
        N_workcenter = n;
    }
    int get_N_workcenter()
    {
        return (N_workcenter);
    }

    void tam_N_maq(int tam)
    {
        N_maq = new int[tam];
    }
    void put_N_maq(int v, int i)
    {
        N_maq[i] = v;
    }
    int* get_N_maq()
    {
        return (N_maq);
    }
    int get_N_maq(int i)
    {
        return (N_maq[i]);
    }

    void tam_list_speeds1(int tam)
    {
        list_speeds = new int*[tam];
    }
    void tam_list_speeds2(int tam, int i)
    {
        list_speeds[i] = new int[tam];
    }
    void put_list_speeds(int v, int i, int j)
    {
        list_speeds[i][j] = v;
    }
    int** get_list_speeds()
    {
        return (list_speeds);
    }
    int get_list_speeds(int i, int j)
    {
        return (list_speeds[i][j]);
    }
};

#endif
