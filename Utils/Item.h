//
// Created by filippocasari on 29/05/21.
//

#ifndef PROJECTPUBSUBZMQ_ITEM_H
#define PROJECTPUBSUBZMQ_ITEM_H


#include <czmq.h>
#include <string>

struct Item_t{
    zmsg_t *msg = zmsg_new();
    long timestamp =0;
    std::string name_of_metric;
};

Item_t *
Item_new( zmsg_t *mex, long ts,char *name)
{
    auto *self = (Item_t *) malloc (sizeof (Item_t));
    self->msg = zmsg_new();
    if(mex==nullptr && ts == 0 && name==nullptr){
        self->msg = zmsg_new();
        self->timestamp=0;

    }else{
        self->msg=mex;
        self->timestamp=ts;
        self->name_of_metric= name;
    }



    return self;
}

#endif //PROJECTPUBSUBZMQ_ITEM_H
