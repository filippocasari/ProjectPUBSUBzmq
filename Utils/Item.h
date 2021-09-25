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
        int64_t ts_start{};
        int64_t num{};
        int64_t ts_end{};
        Item() = default;
        Item(const int64_t startTime, const int64_t *endTime, std::string nameMetric, const int64_t *numMessage) {
            ts_start =startTime;
            ts_end = *endTime;
            name_metric = (std::move(nameMetric));
            num=*numMessage;
        }
        Item(Item const &it){
            this->name_metric=it.name_metric;
            this->ts_end=it.ts_end;
            this->ts_start=it.ts_start;
            this->num=it.num;
        }

};
#endif //PROJECTPUBSUBZMQ_ITEM_H
