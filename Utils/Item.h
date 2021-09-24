//
// Created by filippocasari on 01/06/21.
//

#ifndef PROJECTPUBSUBZMQ_ITEM_H
#define PROJECTPUBSUBZMQ_ITEM_H

#include <czmq.h>
#include <string>
#include <utility>

class Item {
        public:
        std::string name_metric;
        int64_t ts_start;
        int64_t num{};
        int64_t ts_end{};
        Item() = default;
        Item(const int64_t *start, const int64_t *end, std::string name, const int64_t *count) {
            ts_start =*start;
            ts_end = *end;
            name_metric = (std::move(name));
            num=*count;
        }
        Item(Item const &it){
            this->name_metric=it.name_metric;
            this->ts_end=it.ts_end;
            this->ts_start=it.ts_start;
            this->num=it.num;
        }

};
#endif //PROJECTPUBSUBZMQ_ITEM_H
