#pragma once
#define LOG(str) \
        cout<<__FILE__<<":"<<__LINE__<<" "<<__TIMESTAMP__<<endl;\
        cout << str << endl;