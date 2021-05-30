//
// Created by filippocasari on 29/05/21.
//

#ifndef PROJECTPUBSUBZMQ_ITEM_H
#define PROJECTPUBSUBZMQ_ITEM_H


#include <czmq.h>
#include <string>
#include <utility>

/*
struct Item_t{
    zmsg_t *msg;
    long timestamp =0;
    std::string name_of_metric;
};

Item_t *
Item_new( zmsg_t *mex, long ts,char *name)
{
    auto *self = (Item_t *) malloc (sizeof (Item_t));

    self->msg = mex;
    self->timestamp = ts;
    self->name_of_metric = name;
    return self;
}
Item_t *
Item_init(){
    auto *self = (Item_t *) malloc (sizeof (Item_t));
    return self;
}*/
class Item {
public:

    std::string name_metric;
    zmsg_t *msg=zmsg_new();
    long timestamp{};

    Item(zmsg_t *mex, long ts, std::string name) {
        msg = mex;
        timestamp = ts;
        name_metric = std::move(name);
    }

    Item() = default;
};

#endif //PROJECTPUBSUBZMQ_ITEM_H
