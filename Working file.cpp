姑且1.0了吧。
下一个更新，上行下行的时候的加速，如果有可能的话。
质数排序的时候，大的去0，小的去length-1。另外就是剪枝那个条件应该有问题。
时间是O(V^(E/V))...
/*
最新细节：
used是0层写除了0层以外的其他层，然后1写除了0和1的其他，自己不写自己。
最大可能长度要单独维护，不能靠used去推算。
最终的可肉眼读的结果从path上去反推。
curr_len 只对应 path 对应的部分的长度。
（？？？但是used对应的是path加pos覆盖的部分，但是当path达到最大长度的时候，也就是bottom的地方，pos覆盖的是不加入到used里面的？？？）。used只标记客体，不标记本体，比如在第2行，选择了第4个索引，索引的值是10，那么used[1]不动，used[9] = true;这个的意义是data[1]和data[9]加起来是质数，当前被加入选中。变化前，path长度是1，pos是3，变化后，path长度是2，pos是-1.
*/

/*一些笔记。
1,底部和非底部的行为是不同的。也就是说，第一层的判断是判断是否是底部。
2，used只会导致真的vector的部分被锁，而-1那个逻辑位置是无敌的。
3，早pruning的时机是从-1深入，因为从-1深入会浪费这一层所代表的一个数字。可以通过检测used的对应位置来判断。
4，非底部的真实vector部分，要检测想要的那个index的used的情况，如果别人已经用过了，那就不能再用了。最后pos == end的类似操作，然后上升，+1，continue。
5，底部的情况基本是对的，需要两个used没有被占用，而且还要有真实的vector。因为这个vector最多只有1个，所以就是[0]，3个条件同时有，就必然可以得到一对，没有就没有这一对。无论有没有最后一对，都要检测是否比前面的记录长。正常来说，要么持平，要么更长，不够长的话，应该提前剪枝了，根本进不来。
6，最终的使用的数字，无论是具体内容还是总个数，可以从used来，可以从path来（最后一个肯定是0就不用pos了）。办法还是很多的。
*/

/*
* Documentation
pair triangle.
The example is with 2, 5, 6, 13.
Basic idea looks like:

   |  2  5  6  13
2  |  x  7  x  x
5  |     x  11 x
6  |        x  19
13 |           x

x for non-prime.
The diagnal is needed only for 1+1. But it's ok. It doesn't slow down too much.

Then, the whole thing transforms into a new table. It looks like:
2 | 5
5 | 6
6 | 13
Example is too small, if input is [1,2,3,4,5,6], then this table should look like:
1|1,2,4,6
2|3,5           ...
3|4             (<=3 elements)
4|Nothing here. (<=2 elements)
5|6             (this one has either 1 or 0 element)
6|              (this one is always empty. No need to keep this one.
Notice that, 1|1,2,4,6 means 4 pairs, 1|1, 1|2, 1|4 and 1|6.
If(1|2) is choosen, then both 1 and 2 are out. Then the next search starts from 3|4.
This algo maybe handle duplicated numbers.
*/

/*
PC_table[0]   path[0]   path[0]   path[0]
PC_table[1]   pos(int)  path[1]   path[1]
PC_table[2]             pos(int)  path[2]
PC_table[3]                       pos(int)
left_line_count: 2         1         0    this is the same as possible max count from what's left.
when max is 1:   ok        ok     early pruning
*/

//do the bottom job, to parent to next, continue; 
//I optimized this little part. If the last row is not used and it not empty, them it guaruatees to provide 1 and only 1 pair. Other wise it doesn't provide any pair at all. I add the first one of this row to the debug info and directly skip the whole row. 
/*
Then, in here, the memory looks like:
---------------------------------------------
data[0]   PC_table[0]   path[0]    used[0](useless)   maybe buffed
data[1]   PC_table[1]   path[1]    used[1](useful)    maybe buffed
data[2]   PC_table[2]   pos(int)   used[2](useful)    not buffed
data[3]                 ???        used[3](useful)    not buffed

_____________________________________________
???is the position it needs no searching.
Be careful when using their size().
Also, the last PC_table([2]here) has no more than 1 element.
I'm gonna take advantage of this and try optimizing it a bit.


CORRECTION:
the used flags may contain incorrect info. Only used[path.size()] and used[path.size()+n] are ganrunteed to be correct.
The random part is not used at all.

*/


#include<iostream>
#include<vector>
#include<string>
#include<algorithm>
#include<unordered_map>
//using namespace std;
bool DUMMY = false;
#define DEBUG

std::vector<int> PrimeGen(int upper_bound = 30000 * 2) {
   std::vector<int> result{ 2,3 };//at least an odd number.
   std::vector<int> new_result{};

   int last{}, from{}, to_included{};
   bool last_round = false;
   while(1){
      last_round = false;

      last = (*result.rbegin());
      from = last + 2;
      to_included = last + 2;
      to_included = to_included * to_included - 2;
      if (to_included > upper_bound) {
         to_included = upper_bound;
         last_round = true;
      }

      bool prime{ true };
      for (int test_this = from; test_this <= to_included; test_this += 2) {

         prime = true;
         for (int div : result) {
            if (test_this % div == 0) {
               prime = false;
               break;
            }
         }
         if (prime) {
            new_result.push_back(test_this);
         }
      }
      auto old_size = result.size();
      result.resize(result.size() + new_result.size());
      std::copy(new_result.begin(), new_result.end(), result.begin() + old_size);
      new_result.clear();

      if (last_round) { break; }
   }
   return result;
}

class PairingAlgo {
public:
   std::vector<int>* data{};
   std::vector<std::vector<int> > PC_table{};
   int max_buff_length{};
#ifdef DEBUG
   std::vector<std::vector<int> > __DEBUG_PC{};
#endif // DEBUG


   void SetData(std::vector<int>* p_data) {
      data = p_data;

      //Sorts first.
      std::sort(data->begin(), data->end());//probably <

#ifdef DEBUG
      if (data->empty())throw("jkl;dfsa");
#endif // DEBUG

      //The PRIME
      int upper_bound{ data->back() * 2 };//*2 is very important
      auto PRIME = PrimeGen(upper_bound);
      std::sort(PRIME.begin(), PRIME.end());//probably <

      //The triangle.
      PC_table.clear();
      PC_table.resize(data->size() - 1);//The last one is not needed since it's empty 4ever.
      int small{}, big{};
      // data + PRIME >>> prime companion
      for (unsigned int i1 = 0; i1 < PC_table.size(); i1++) {
         small = data->at(i1);
         for (unsigned int i2 = i1; i2 < data->size(); i2++) {
            big = data->at(i2);
            bool _r = std::binary_search(PRIME.begin(), PRIME.end(),
               (small + big));
            if (_r) {
               PC_table[i1].push_back(i2);//This is very important. If the concrate number is stored, I don't know how to index. I don't care the concrate number, I need only the index. But if your design is based on storing the number than index, remember to modify here as well.
            }
         }
      }
   }//void SetData(std::vector<int>* p_data) 
  
   std::vector<bool> used{};
   std::vector<int> path{};
   int pos{ -1 };

   int ____max_possible_left() {
      auto r = std::count(used.begin() + path.size(), used.end(), false);
      return static_cast<int>(r);
   }

   int curr_len{ 0 };

   /*If the func returns true, call go next !!!!*/
   void _to_child_or_EP____continue() {
      if (-1 == pos) {
         //When skips any number, it may get into a use less branch. Tests early pruning first. If it's still worthy, update path and pos and go on. If it's not worthy any more, call to next and return;
         int possible_max_length = curr_len + ____max_possible_left();
         bool needs_pruning = {max_buff_length >= possible_max_length};
         if(needs_pruning){//Do the pruning. 
            _to_next_unused();
         }
         else {
            //Goes deeper with a skip. No number is choosen.
            path.push_back(pos);
            //pos = -1; pos is already -1
         }
      }
      else{
         used[curr_index()] = true;
         curr_len += 2;
         //buff__DDDD.push_back(static_cast<int>(path.size()));
         //buff__DDDD.push_back(curr_index());
         path.push_back(pos);
         pos = -1;
      }
      //returning_probably_wrong = false;
      return  ;
   }

   /*Notice: This function is only called at 1 case, where the pos is always at some end-like position.*/
   /*return true for time to break out of the main loop.*/
   bool _to_parent() {
#ifdef DEBUG
      //if (pos != curr_row().size()) throw("dfsjkldfsjkl"); Let's test it later.
#endif // DEBUG

      if (path.empty())
      {//The loop should totally end here, but how can I break from a func?
         return true;
      }

      else {
         /*
         I think I only have 2 cases to handle here.
         1, the job is from early pruning, then, it should be -1 to -1 case. No used flag is touched at all.
         2, the job is from end-like pos. The path.back() could be anything. The only used flag has to be concerned is the one pointed by the path.back().
         The branch could only care about the value of pos.
         */
         if (pos < 0) {//early pruning as mentioned.
            //pos is still -1, no need to assign to it.
            throw("probably dead branch.....");
            path.pop_back();
         }
         else/*This else is very important.*/ {//pos is end-like, path.back could be anything.
            pos = path.back();//Old pos is always end-like, no used flag needs to be set.
            path.pop_back();
            if (pos >= 0) {//This pos should never be end-like, otherwise how did it go deeper. The only exception is the skipping -1.
               used[curr_index()] = false;//This should always be safe.
               curr_len -= 2;
            }
         }
         return false;
      }
   }
 //        if (pos >= 0 && !pos_too_big_logical()) {
 //           throw ("This is probably a dead branch. If it's fired, emmm.");
 //           used[curr_index()] = false;
 //           //buff__DDDD.resize(buff__DDDD.size() - 2);
 //        }
 //        pos = path.back();//safe. 
 //        if ((pos >= 0) && ) {
 //           curr_len -= 2;
 //        }
 //        path.pop_back();
 //        return false;
 //     }
 //  }

   void _to_next_unused() {
      //This function works only within the current row, no deeper no shallower, the cr is logically constant.
      if (curr_row_used()) {
         pos = 0;
         return;
      }
      const auto cr = curr_row();
      //Step 1, set the old used flag back to false.
      if (pos != -1) {
         used[cr[pos]] = false;
      }

      //Step 2, find the next available position. If the row is marked used, assign pos to 0. Otherwise, try searching for an available position, or end-like as convention.
      
      auto fr = std::find_if(cr.begin() + (pos + 1), cr.end(), [this](const int e) {return (!used[e]); });
      if (fr != cr.end()) {
         //Found a valid position. Updates it.
         pos = static_cast<int>(fr - cr.begin());
         used[cr[pos]] = true;
      }
      else
      {//Nothing available any more. Sets pos to the end-like position.
         pos = static_cast<int>(cr.size());
      }
      return;
   }



 //     //step 2 to next
 //     if (-1 == pos) {
 //        //from -1, means didn't use any number. If reaches non-end-like, then needs some number, but if reaches /end-/like, no number is needed.
 //        ++pos;
 //        if (curr_row().empty()) {
 //           return ;
 //        }
 //        else
 //        {
 //           //used[path.size()] = true;xxxxxxxxxxxx
 //           used[curr_index()] = true;
 //           //path.push_back(pos);xxxxxxxxxx
 //           //buff__DDDD.push_back(static_cast<int>(path.size()));
 //           //buff__DDDD.push_back(curr_index());
 //           //curr_len += 2;xxxxxxxxcurr len covers only what the path corresponds.
 //           return ;
 //        }
 //     }
 //     else {
 //        //from non -1 position. Release old used mark first, then non-end-like or end-like.
 //  /*
 //  memory looks like:         before trick  in   out                        
 //     x  mark for current row   t      t    t     -  (probably should not touch this one)            
 //                 guest row 1   t      -    -     -                        
 //                 guest row 2   -      -    t     -                        
 //  */
 //        used[curr_index()] = false;
 //        //trick here
 //        ++pos;
 //        if (!pos_too_big_logical()){
 //           used[curr_index()] = true; 
 //        }
 //        return ;
 //     }
 //     return ;//this func returns false to mean the main loop should go on. Go on, not a continue of c++.
 //  };

   //bool pos_too_big() {
   //   return pos >= PC_table[path.size()].size();
   //}
   bool pos_too_big_logical() {
      if (curr_row_used()){return pos >= 0;}
      int curr_row_size = static_cast<int>(curr_row().size());
      return (pos >= curr_row_size);
   }

   inline const bool curr_row_used() {return used[path.size()];}
   inline bool curr_pos_used() { return used[curr_index()]; }
   inline std::vector<int> curr_row() {return PC_table[path.size()];}
   inline int curr_index() {
      return curr_row()[pos];
   }

   /*If the row is used, it behaviors the same as empty.*/
   //bool _test_element____dddd() {
   //   if (curr_row_used()) { return false; }
   //   if(pos >= static_cast<int>(curr_row().size()) ) { return false; }
   //}
   //int logic_row_size____DDDD() {
   //   if (curr_row_used()) { return 0; }
   //   return static_cast<int>(curr_row().size());
   //}

   inline bool is_bottom() {
      /*
         data[0]   PC_table[0]   path[0]    used[0](useless)   
         data[1]   PC_table[1]   path[1]    used[1](useful)    
         data[2]   PC_table[2]   pos(int)   used[2](useful)    
         data[3]      N/A                   used[3](useful)    
      */
      auto r = PC_table.size() <= (path.size() + 1);
      return r;
   }
   void _job_at_bottom____continue() {
      bool ____score_at_bottom = this->__does_score_at_bottom();
      int path_non_neg_1 = static_cast<int>(std::count_if(path.begin(), path.end(), [](int e) {return e >= 0; }));

      int new_length = (path_non_neg_1 + ____score_at_bottom) * 2;
     //+0 for false and +2 for true.
      if (max_buff_length < new_length) {
         max_buff_length = new_length;
#ifdef DEBUG
         if (____score_at_bottom) {
            path.push_back(pos);//The last element of PC_table is tested in __does_score_at_bottom(). If ____score_at_bottom is true, the last element of PC_table contains one and only one element.
         }
         __DEBUG_PC.push_back(path);
         if (____score_at_bottom) {
            path.pop_back();
         }
#endif // DEBUG
      }
      //With these few lines, this function handle its own case. So that the _to_parent function is not polluted.
      pos = path.back();
      path.pop_back();
      if (pos >= 0)/*pos here can never be the end-like position. Otherwise it couldn't have come inside here.*/ {
      //   used[curr_row()[pos]] = false;
         curr_len -= 2;
      }
      _to_next_unused();
      return;
   }
   inline bool __does_score_at_bottom() {
      bool used_1 = *used.rbegin();
      if (used_1) { return false; }
      bool used_2 = *(used.rbegin()+1);
      if (used_2) { return false; }
      bool table_em = PC_table.rbegin()->empty();
      if (table_em) { return false; }
      return true;
   }

   void Run() {
#ifdef DEBUG
      __DEBUG_PC.clear();
#endif // DEBUG
      //Safety first
      if (1 == PC_table.size()) {
         if (PC_table[0].empty()) {
            max_buff_length = 0;
            return;
         }
         else{
            max_buff_length = 2;
#ifdef DEBUG
            __DEBUG_PC.push_back(std::vector<int>{ 0 });
#endif // DEBUG
            return;
         }
      }
      if (PC_table.empty()) {
         max_buff_length = 0;
         return;
      }

      //Step 1, greedly searching. This searching is likely to provide some min possible answer for the whole task.
      max_buff_length = 0;
      used.resize(this->data->size());std::fill(used.begin(), used.end(), false);
#ifdef DEBUG
      __DEBUG_PC.clear();
      __DEBUG_PC.resize(1);
#endif // DEBUG
      for (unsigned int i = 0; i < PC_table.size(); ++i)
      {
         auto row = PC_table[i];
         if (!used[i] && !row.empty() && !used[row[0]]/*put used[row[0]] in the end to make it safe.*/) {
            used[row[0]] = true;
            max_buff_length += 2;
#ifdef DEBUG
            __DEBUG_PC[0].push_back(row[0]);
#endif // DEBUG
         }
      }
#ifdef DEBUG
      if (!__DEBUG_PC[0].empty()) { __DEBUG_PC.clear(); }
#endif // DEBUG

      //Step 2 THE LOOP
      curr_len = 0;
      path.clear();
      pos = -1;
      used.resize(this->data->size()); std::fill(used.begin(), used.end(), false);

      int debug_count = 0;
      // THE LOOP!!!
      while (1) {
         debug_count += 1;
         if (1 <= debug_count) {
            int dfjkls = 345789;
         }

         if (is_bottom()) {
            _job_at_bottom____continue();//This function handles some to parent logic out side the dedicated _to_parent function.
            continue;
         }
         else{
            /*step the loop forward, like normal infi loop for graphs.
               if the step is "goes deeper from a -1", then it needs to test early pruning.*/
            if (pos_too_big_logical()) {
               if(_to_parent()) { break; }
               _to_next_unused();
               continue;
            }
            else {
               _to_child_or_EP____continue();
               continue;
            }
         }
      }//while(1)
   };//void Run() {

   int GetMaxPairCount()const { return max_buff_length / 2; }
};

int test() {
   std::vector<int> data{2,5,8,12,16,22};//Pure dummy.

   PairingAlgo pa2{};
   pa2.SetData(&data);
   pa2.PC_table.clear();
   pa2.PC_table.push_back(std::vector<int>{1, 2, 5});
   pa2.PC_table.push_back(std::vector<int>{2});
   pa2.PC_table.push_back(std::vector<int>{3});
   pa2.PC_table.push_back(std::vector<int>{4});
   pa2.PC_table.push_back(std::vector<int>{});//Only 5. The last one is empty.
   pa2.used.resize(6);
   std::fill(pa2.used.begin(), pa2.used.end(), false);
   pa2.data->resize(6);//bad idea... but works.
   pa2.Run();

   //res[i] >>> data[i]   AND   data[PC_table[i][res[i]]

   return 0;
}
int test_empty() {
   std::vector<int> data{ 3,6 };//Pure dummy.

   PairingAlgo pa2{};
   pa2.SetData(&data);
   pa2.Run();

   //res[i] >>> data[i]   AND   data[PC_table[i][res[i]]

   return 0;
}
int test_9() {
   std::vector<int> data{ 10602 ,19214 ,20156 ,17292 ,12367 ,21563 ,20894 ,13208 ,29619 ,15607 ,15373 ,1876 ,13013 ,13288 ,6086 ,17433 ,29272 ,27894 ,28354 ,2472 ,17543 ,20939 ,18768 ,28249 ,28372 ,18370 ,17803 ,4997 ,10176 ,9037 ,1197 ,27966 ,20715 ,22939 ,21846 ,12835 };//Pure dummy.

   PairingAlgo pa2{};
   pa2.SetData(&data);
   pa2.Run();

   //res[i] >>> data[i]   AND   data[PC_table[i][res[i]]

   return 0;
}

int main() {
   test_9();
   return 0;
   //DUMMY = true;//Remove this line when copy into LeetCode.////////////////////////////////////////////////////////////////////////////////////////////////
   std::vector<std::string> INPUT{};
   if (!DUMMY) {
      std::string temp_string;
      std::cin >> temp_string;
      auto data_len = std::stoul(temp_string);
      while (std::cin)
      {
         std::cin >> temp_string;
         INPUT.push_back(temp_string);
      }
      INPUT.resize(data_len);
   }
   else {
      //INPUT.push_back("4");
      INPUT.push_back("2");
      INPUT.push_back("5");
      INPUT.push_back("6");
      INPUT.push_back("13");
   }

   std::vector<int> data{};
   data.resize(INPUT.size());
   std::transform(INPUT.begin(), INPUT.end(), data.begin(), [](auto s) {return std::stoi(s); });
  
   PairingAlgo pa{};
   pa.SetData(&data);
   pa.Run();
   int result = pa.GetMaxPairCount();
   std::cout << result << std::endl;

   return 0;
}



