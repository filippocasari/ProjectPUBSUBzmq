//
// Created by Filippo Casari on 09/09/21.
//

#include <czmq.h>
#include "PUB.h" // import our Publisher
#include "SUB.h" // import our Subscriber
using namespace std;
#define NUM_SUB 7 // number of subscribers

// main function, let's play
int main(int argc, char **argv) {
    //vector<thread> SUBS; // if you want working with thread of c++
    //SUBS.reserve(NUM_SUB); // reserve "n" subscriber threads

    zactor_t *actors[NUM_SUB]; // working with zactors from
    // ###### NOTE THAT Z-ACTORS WANTS A SINGLE ARG #############

    string comma = ","; // just a comma to link arguments
    string comma2 ="&"; // just a & to link arguments
    // THE (DUMB) IDEA IS TO LINK ARG[1], ARG[2] and ARG[3] with the above symbols
    // The final string passed must be: path of file Json + "," + path of Csv + "&" + verbose
    // This rule can definitely be changed, it is awful

    // starting new thread PUB. Must be initialized before the SUB, because Sync Service is implemented
    // and PUB must listen on it. SUB will send their messages for sync to the PUB.
    thread start_pub([&argc, &argv]{
        main_PUB(argc, argv);
    });

    zclock_sleep(3000); // just sleep few secs to wait PUB configuration

    char *temp_char; // will be used to store path csv + (number of Subscriber)


    // Let's start our loop to create Subscribers
    for (int i = 0; i < NUM_SUB; i++) {
        string temp;
        temp = ((char *)argv[2]); // takes path of csv and store it in a new variable
        string final_string;

        string *i_str; // string to store "_"+i
        *i_str= "_" ;
        i_str->append(to_string(i));
        temp=(argv[2]);
        temp.append(*i_str);

        // Let's concatenate strings
        // initialize string to first arg
        final_string = (char *)argv[1];// final string, used to pass every args to z-actors
        final_string.append(comma); // append comma
        final_string.append(temp); // append second arg
        final_string.append(comma2); // append &
        final_string.append((char *)argv[3]); // append last arg
        cout<<"FINAL STRING: "<<final_string<<endl; // print our final string

        // create new z-Actor, it is a thread
        actors[i]= zactor_new(main_SUB_M, (void *) &final_string);

        sleep(1); // sleep for a while, just to configure our Sub
    }

    // join Pub thread
    start_pub.join();

    // short for loop to join our SUB threads
    for(auto & actor : actors){
        zactor_destroy(&actor);
    }

    return 0;
}