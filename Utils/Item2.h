//
// Created by filippocasari on 01/06/21.
//

#ifndef PROJECTPUBSUBZMQ_ITEM2_H
#define PROJECTPUBSUBZMQ_ITEM2_H

#include <czmq.h>
#include <string>
#include <utility>

class Item2 {
        public:
        std::string name_metric;
        std::string ts_start;
        int64_t ts_end{};
        Item2() = default;
        Item2(std::string mex, int64_t ts, std::string name) {
            ts_start = std::move(mex);
            ts_end = ts;
            name_metric = std::move(name);
        }
        Item2(Item2 const &it){
            this->name_metric=it.name_metric;
            this->ts_end=it.ts_end;
            this->ts_start=it.ts_start;
        }

};
#endif //PROJECTPUBSUBZMQ_ITEM2_H
