#ifndef FILE_GANTT_H
#define FILE_GANTT_H

class Input_File_gantts_p1
{
    int n_jobs;
    int* N_opers;
    int* r;
    int* d;
    int* w;
    int* c;

public:
    void put_n_jobs(int v)
    {
        n_jobs = v;
    }
    int get_n_jobs()
    {
        return (n_jobs);
    }

    // metodos N_opers

    void tam_N_opers1(int tam)
    {
        N_opers = new int[tam];
    }
    void put_N_opers(int v, int i)
    {
        N_opers[i] = v;
    }
    int* get_N_opers2()
    {
        return (N_opers);
    }
    int get_N_opers(int i)
    {
        return (N_opers[i]);
    }

    // metodos release

    void tam_r1(int tam)
    {
        r = new int[tam];
    }
    void put_r(int v, int i)
    {
        r[i] = v;
    }
    int* get_r2()
    {
        return (r);
    }
    int get_r(int i)
    {
        return (r[i]);
    }

    // metodos due date

    void tam_d1(int tam)
    {
        d = new int[tam];
    }
    void put_d(int v, int i)
    {
        d[i] = v;
    }
    int* get_d2()
    {
        return (d);
    }
    int get_d(int i)
    {
        return (d[i]);
    }

    // metodos weight

    void tam_w1(int tam)
    {
        w = new int[tam];
    }
    void put_w(int v, int i)
    {
        w[i] = v;
    }
    int* get_w2()
    {
        return (w);
    }
    int get_w(int i)
    {
        return (w[i]);
    }

    // metodos completion time

    void tam_c1(int tam)
    {
        c = new int[tam];
    }
    void put_c(int v, int i)
    {
        c[i] = v;
    }
    int* get_c2()
    {
        return (c);
    }
    int get_c(int i)
    {
        return (c[i]);
    }
};

class Input_File_gantts_p2
{
    int workcenters;
    int* num_machines;
    int* opers_mac;
    int** job;
    int** step;
    int** inicio;
    int** fin;

public:
    // metodos workcenters

    void put_workcenters(int v)
    {
        workcenters = v;
    }
    int get_workcenters()
    {
        return (workcenters);
    }

    // metodos opers_mac

    void tam_opers_mac1(int tam)
    {
        opers_mac = new int[tam];
    }
    void put_opers_mac(int v, int i)
    {
        opers_mac[i] = v;
    }
    int* get_opers_mac2()
    {
        return (opers_mac);
    }
    int get_opers_mac(int i)
    {
        return (opers_mac[i]);
    }

    // metodos num_machines

    void tam_num_machines1(int tam)
    {
        num_machines = new int[tam];
    }
    void put_num_machines(int v, int i)
    {
        num_machines[i] = v;
    }
    int* get_num_machines2()
    {
        return (num_machines);
    }
    int get_num_machines(int i)
    {
        return (num_machines[i]);
    }

    // metodos job

    void tam_job1(int tam)
    {
        job = new int*[tam];
    }
    void tam_job2(int tam, int i)
    {
        job[i] = new int[tam];
    }

    void put_job(int v, int i, int j)
    {
        job[i][j] = v;
    }

    int** get_job2()
    {
        return (job);
    }
    int get_job(int i, int j)
    {
        return (job[i][j]);
    }

    // metodos step

    void tam_step1(int tam)
    {
        step = new int*[tam];
    }
    void tam_step2(int tam, int i)
    {
        step[i] = new int[tam];
    }

    void put_step(int v, int i, int j)
    {
        step[i][j] = v;
    }

    int** get_step2()
    {
        return (step);
    }
    int get_step(int i, int j)
    {
        return (step[i][j]);
    }

    // metodos inicio

    void tam_i1(int tam)
    {
        inicio = new int*[tam];
    }
    void tam_i2(int tam, int i)
    {
        inicio[i] = new int[tam];
    }

    void put_i(int v, int i, int j)
    {
        inicio[i][j] = v;
    }

    int** get_i2()
    {
        return (inicio);
    }
    int get_i(int i, int j)
    {
        return (inicio[i][j]);
    }

    // metodos fin

    void tam_f1(int tam)
    {
        fin = new int*[tam];
    }
    void tam_f2(int tam, int i)
    {
        fin[i] = new int[tam];
    }

    void put_f(int v, int i, int j)
    {
        fin[i][j] = v;
    }

    int** get_f2()
    {
        return (fin);
    }
    int get_f(int i, int j)
    {
        return (fin[i][j]);
    }
};

#endif
