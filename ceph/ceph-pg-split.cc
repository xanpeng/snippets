// 
// About Ceph pg splitting, determine what should pg .17 be splitted to. 
//
#include <iostream>
 
using namespace std;
 
int calc_bits_of(int t) {
  int b = 0;
  while (t > 0) {
    t = t >> 1;
    b++;
  }
  return b;
}
 
int ceph_stable_mod(int x, int b, int bmask) {
  if ((x & bmask) < b)
    return x & bmask;
  else
    return x & (bmask >> 1);
}
 
bool is_split(int old_pg_num, int new_pg_num, int m_seed) {
  if (new_pg_num <= old_pg_num) {
    cout << "new_pg_num <= old_pg_num\n";
    return false;
  }
 
  bool split = false;
 
  int old_bits = calc_bits_of(old_pg_num);
  int old_mask = (1 << old_bits) - 1;
  cout << "old_bits: " << old_bits << ", old_mask: " << old_mask << endl;
  for (int n = 1; ; n++) {
    int next_bit = (n << (old_bits - 1));
    int s = next_bit | m_seed;
 
    if (s < old_pg_num || s == m_seed)
      continue;
    if (s >= new_pg_num)
      break;
 
    if (ceph_stable_mod(s, old_pg_num, old_mask) == m_seed) {
      cout << "next_bit: " << next_bit << ", s: " << s << " (hex: " << hex << s << ")" << dec << endl;
      split = true;
    }
  }
 
  return split;
}
 
int get_ancestor(int m_seed, int old_pg_num) {
  int old_bits = calc_bits_of(old_pg_num);
  int old_mask = (1 << old_bits) - 1;
  return ceph_stable_mod(m_seed, old_pg_num, old_mask);
}
 
int main() {
  unsigned old_pg_num;
  unsigned new_pg_num;
  int m_seed;
  cout << "old_pg_num: ";
  cin >> old_pg_num;
  cout << "new_pg_num: ";
  cin >> new_pg_num;
  cout << "m_seed: ";
  cin >> m_seed;
  cout << "is_split(" << old_pg_num << ", " << new_pg_num << ", " << m_seed << ")? --"
    << is_split(old_pg_num, new_pg_num, m_seed) << endl;
 
  /*
  cout << "get_ancestor of: ";
  cin >> m_seed;
  cout << "old_pg_num: ";
  cin >> old_pg_num;
  cout << "ancestor: ";
  cout << get_ancestor(m_seed, old_pg_num) << endl;
  */
}
