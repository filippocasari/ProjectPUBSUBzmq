//
// Created by filippocasari on 01/06/21.
//

#ifndef PROJECTPUBSUBZMQ_ITEM2_H
#define PROJECTPUBSUBZMQ_ITEM2_H

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
        Item() = default;
        explicit Item(Item *pItem) {
            this->name_metric=pItem->name_metric;
            this->timestamp=pItem->timestamp;
            this->msg=pItem->msg;
        }

        Item(zmsg_t *mex, long ts, std::string name) {
            msg = mex;
            timestamp = ts;
            name_metric = std::move(name);
        }
        Item(const Item& item){
            msg=item.msg;
            timestamp=item.timestamp;
            name_metric=item.name_metric;
        }


};
#endif //PROJECTPUBSUBZMQ_ITEM2_H
