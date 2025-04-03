#include <bits/stdc++.h>
// #include "lcgrand.h"
#define MODLUS 2147483647
#define MULT1       24112
#define MULT2       26143

using namespace std;

int amount, bigs, initial_inv_level, inv_level, next_event_type, num_events, num_months, num_values_demand, smalls;

float area_holding, area_shortage, holding_cost, incremental_cost, maxlag, mean_interdemand, minlag, prob_distrib_demand[25], setup_cost, shortage_cost, sim_time, time_last_event, time_next_event[5], total_ordering_cost;

FILE *infile, *outfile;

void initialize(void);
void timing(void);
void order_arrival(void);
void demand(void);
void evaluate(void);
void report(void);
void update_time_avg_stats(void);
float expon(float mean);
int random_integer(float prob_distrib[]);
float uniform(float a, float b);
float lcgrand(int stream);

int main(){

    int i, num_policies;

    /* Open input and output files */

    infile = fopen("in.txt", "r");
    outfile = fopen("inv.txt", "w");

    /* Specify the number of events for the timing function. */

    num_events = 4;

    cout << "Heloo" << endl;

    /* Read input parameters. */
    fscanf(infile, "%d %d %d %d %f %f %f %f %f %f %f", &initial_inv_level, &num_months, &num_policies, &num_values_demand, &mean_interdemand, &setup_cost, &incremental_cost, &holding_cost, &shortage_cost, &minlag, &maxlag);

    for(i = 1; i <= num_values_demand; ++i){
        fscanf(infile, "%f", &prob_distrib_demand[i]);
    }


    cout << "Heloo1" << endl;
    /* Write report heading and input parameters. */

    fprintf(outfile, "Single-product inventory system\n\n");
    fprintf(outfile, "Initial inventory level%24d items\n\n", initial_inv_level);
    fprintf(outfile, "Number of demand sizes%23d\n\n", num_values_demand);
    fprintf(outfile, "Distribution function of demand sizes  ");
    for(i = 1; i <= num_values_demand; i++){
        fprintf(outfile, "%8.3f", prob_distrib_demand[i]);
    }
    fprintf(outfile, "\n\nMean interdemand time%26.2f months\n\n", mean_interdemand);
    fprintf(outfile, "Delivery lag range%29.2f to%10.2f months\n\n", minlag, maxlag);
    fprintf(outfile, "Length of simulation%23d months\n\n", num_months);
    fprintf(outfile, "K =%6.1f i =%6.1f h =%6.1f p =%6.1f\n\n", setup_cost, incremental_cost, holding_cost, shortage_cost);
    fprintf(outfile, "Number of policies%29d\n\n", num_policies);
    fprintf(outfile, "                 Average          Average");
    fprintf(outfile, "          Average      Average\n");
    fprintf(outfile, " Policy         total cost     ordering cost  ");
    fprintf(outfile, " holding cost     shortage cost");

    /* Run the simulation varying the inventory policy. */

    for(i = 1; i <= num_policies; ++i){

        /* Read the inventory policy, and initialize the simulation */

        fscanf(infile, "%d %d", &smalls, &bigs);
        initialize();

        /* Run the simulation until it terminates after an end-simulation event(type 3) occurs. */

        do {

            /* Determine the next event */

            timing();

            /* Update the time-average statistical accumulators. */

            update_time_avg_stats();
            
            /* Invoke the appropriate event function. */

            switch(next_event_type){
                case 1:
                    order_arrival();
                    break;
                case 2:
                    demand();
                    break;
                case 4:
                    evaluate();
                    break;
                case 3:
                    report();
                    break;
            }

            /* If the event just executed was not the end-simulation event (type 3), continue simulating. Otherwise, end the simulation for the current (s,S) pair and go on to the next pair (if any) */

        } while(next_event_type != 3);
    }

    cout << "Heloo2" << endl;

    /* End the simulations */

    fclose(infile);
    fclose(outfile);

    return 0;

}


void initialize(void){

    /* Initialize the simulation clock. */

    sim_time = 0.0;

    /* Initialize the state variables. */

    inv_level = initial_inv_level;
    time_last_event = 0.0;

    /* Initialize the statistical counters. */

    total_ordering_cost = 0.0;
    area_holding = 0.0;
    area_shortage = 0.0;

    /* Initialize the event list. Since no customers are present, the departure (type 2) event is eliminated from consideration. */

    time_next_event[1] = 1.0e+30;
    time_next_event[2] = sim_time + expon(mean_interdemand);
    time_next_event[3] = num_months;
    time_next_event[4] = 0.0;

}


void timing(void){

    int i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0;

    /* Determine the event type of the next event to occur. */

    for(i = 1; i <= num_events; i++){
        if(time_next_event[i] < min_time_next_event){
            min_time_next_event = time_next_event[i];
            next_event_type = i;
        }
    }

    /* Check to see whether the event list is empty. */

    if(next_event_type == 0){
        /* The event list is empty, so stop the simulation. */
        fprintf(outfile, "\nEvent list empty at time %f", sim_time);
        exit(1);
    }

    /* Update the simulation clock. */

    sim_time = min_time_next_event;

}


void order_arrival(void){

    /*  Increment the inventory level by the amount ordered */

    inv_level += amount;

    /* Since no order is now outstanding, eliminating the order-arrival event from consideration */

    time_next_event[1] = 1.0e+30;

}


void demand(void){

    

    /* Decrement the inventory level by a generated demand size. */

    inv_level -= random_integer(prob_distrib_demand);

    /* Schedule the time of the next demand. */

    time_next_event[2] = sim_time + expon(mean_interdemand);

}


void evaluate(void){

    /* Check whether the inventory level is less than smalls. */

    if(inv_level < smalls){

        /* The inventory level is less than smalls, so place an order for the
            appropriate amount. */

        amount = bigs - inv_level;
        total_ordering_cost += setup_cost + incremental_cost * amount;

        /* Schedule the arrival of the order. */
        time_next_event[1] = sim_time + uniform(minlag, maxlag);

    }

    /* Regardless of the place-order decision, schedule the next inventory
        evaluation. */
    time_next_event[4] = sim_time + 1.0;



}


void report(void) /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */
    float avg_holding_cost, avg_ordering_cost, avg_shortage_cost;

    avg_ordering_cost = total_ordering_cost / num_months;
    avg_holding_cost = holding_cost * area_holding / num_months;
    avg_shortage_cost = shortage_cost * area_shortage / num_months;
    fprintf(outfile, "\n\n(%3d,%3d)%15.2f%15.2f%15.2f%15.2f",
            smalls, bigs,
            avg_ordering_cost + avg_holding_cost + avg_shortage_cost,
            avg_ordering_cost, avg_holding_cost, avg_shortage_cost);

}


void update_time_avg_stats(void) { /* Update area accumulators for time-average
statistics. */
    float time_since_last_event;
    /* Compute time since last event, and update last-event-time marker. */
    
    time_since_last_event = sim_time - time_last_event;
    time_last_event = sim_time;
    /* Determine the status of the inventory level during the previous interval.
        If the inventory level during the previous interval was negative, update
        area_shortage. If it was positive, update area_holding. If it was zero,
        no update is needed. */

    if (inv_level < 0)
        area_shortage -= inv_level * time_since_last_event;
    else if (inv_level > 0)
        area_holding += inv_level * time_since_last_event;

}

int random_integer(float prob_distrib[]){ /* Random integer generation
                                            function. */
 
    int i;
    float u;
    /* Generate a U(0,1) random variate. */
    u = lcgrand(1);
    /* Return a random integer in accordance with the (cumulative) distribution
        function prob_distrib. */
    for (i = 1; u >= prob_distrib[i]; ++i)
        ;
    return i;
}

float uniform(float a, float b) { /* Uniform random number generator. */
    
    /* Return a U(a,b) random variate. */
    
    return a + (b - a) * lcgrand(1);
}

float expon(float mean) {
    // Create a random device to seed the generator
    std::random_device rd;
    
    // Use Mersenne Twister for generating random numbers
    std::mt19937 gen(rd());
    
    // Generate random numbers uniformly in the range [0, 1)
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // Return an exponential random variate with the specified mean
    return -mean * log(dis(gen));
}

/* Set the default seeds for all 100 streams. */

static long zrng[] =
{         1,
 1973272912, 281629770,  20006270,1280689831,2096730329,1933576050,
  913566091, 246780520,1363774876, 604901985,1511192140,1259851944,
  824064364, 150493284, 242708531,  75253171,1964472944,1202299975,
  233217322,1911216000, 726370533, 403498145, 993232223,1103205531,
  762430696,1922803170,1385516923,  76271663, 413682397, 726466604,
  336157058,1432650381,1120463904, 595778810, 877722890,1046574445,
   68911991,2088367019, 748545416, 622401386,2122378830, 640690903,
 1774806513,2132545692,2079249579,  78130110, 852776735,1187867272,
 1351423507,1645973084,1997049139, 922510944,2045512870, 898585771,
  243649545,1004818771, 773686062, 403188473, 372279877,1901633463,
  498067494,2087759558, 493157915, 597104727,1530940798,1814496276,
  536444882,1663153658, 855503735,  67784357,1432404475, 619691088,
  119025595, 880802310, 176192644,1116780070, 277854671,1366580350,
 1142483975,2026948561,1053920743, 786262391,1792203830,1494667770,
 1923011392,1433700034,1244184613,1147297105, 539712780,1545929719,
  190641742,1645390429, 264907697, 620389253,1502074852, 927711160,
  364849192,2049576050, 638580085, 547070247 };

/* Generate the next random number. */

float lcgrand(int stream)
{
    long zi, lowprd, hi31;

    zi     = zrng[stream];
    lowprd = (zi & 65535) * MULT1;
    hi31   = (zi >> 16) * MULT1 + (lowprd >> 16);
    zi     = ((lowprd & 65535) - MODLUS) +
             ((hi31 & 32767) << 16) + (hi31 >> 15);
    if (zi < 0) zi += MODLUS;
    lowprd = (zi & 65535) * MULT2;
    hi31   = (zi >> 16) * MULT2 + (lowprd >> 16);
    zi     = ((lowprd & 65535) - MODLUS) +
             ((hi31 & 32767) << 16) + (hi31 >> 15);
    if (zi < 0) zi += MODLUS;
    zrng[stream] = zi;
    return (zi >> 7 | 1) / 16777216.0;
}


void lcgrandst (long zset, int stream) /* Set the current zrng for stream
                                          "stream" to zset. */
{
    zrng[stream] = zset;
}


long lcgrandgt (int stream) /* Return the current zrng for stream "stream". */
{
    return zrng[stream];
}