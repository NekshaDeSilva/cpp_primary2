#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <set>
#include <map>

using namespace std;

void fast_io() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
}

const int MAXN = 200005;

int N_val,  M_val;
int A[MAXN];
vector<int>  adj_rev[MAXN];
int sz[MAXN];

int vis[MAXN];
int  on_cycle[MAXN];
int root_of[MAXN];
int comp_id[MAXN];
vector<int>   cycles[MAXN];
int cycle_pos[MAXN];
int  cycle_len[MAXN];
int cid_counter = 0;

int tin[MAXN],  tout[MAXN],   timer;
int depth[MAXN],  parent[MAXN];
int top[MAXN];

char  type[MAXN];
long long cnt[MAXN];
long long  ans[128];

long long  available[MAXN];

long long bit_tree[MAXN];

void  update_tree(int idx,   long long val) {
    for (;  idx <= N_val;   idx += idx & -idx)  bit_tree[idx] += val;
}

long long  query_tree(int idx) {
    long long sum  = 0;
    for (;  idx > 0;  idx -= idx & -idx)  sum += bit_tree[idx];
    return  sum;
}

vector<long long>  bit_cycle[MAXN];

void  update_cycle(int c_id,  int idx,   long long val) {
    for (;  idx < bit_cycle[c_id].size();  idx += idx & -idx)   bit_cycle[c_id][idx] += val;
}

long long  query_cycle_bit(int c_id,   int idx) {
    long long sum  = 0;
    for (;  idx > 0;  idx -= idx & -idx)  sum += bit_cycle[c_id][idx];
    return  sum;
}

long long  query_cycle_range(int c_id,   int l,  int r) {
    int len  = cycle_len[c_id];
    if (l > r) {
        return  (query_cycle_bit(c_id,  len) - query_cycle_bit(c_id,  l))  + query_cycle_bit(c_id,  r + 1);
    }  else {
        return  query_cycle_bit(c_id,  r + 1)  - query_cycle_bit(c_id,  l);
    }
}

set<int>  active_cycle_nodes[MAXN];

int  tree_max_depth[4 * MAXN];

void  update_depth(int node,   int start,  int end,  int idx,   int val) {
    if  (start == end) {
        tree_max_depth[node]  = val;
        return;
    }
    int mid  = (start + end) / 2;
    if  (idx <= mid)   update_depth(2 * node,  start,  mid,  idx,  val);
    else   update_depth(2 * node + 1,  mid + 1,  end,  idx,  val);
    tree_max_depth[node]  = max(tree_max_depth[2 * node],   tree_max_depth[2 * node + 1]);
}

int  query_depth(int node,   int start,  int end,  int l,   int r) {
    if  (r < start || end < l)   return 0;
    if  (l <= start && end <= r)   return tree_max_depth[node];
    int mid  = (start + end) / 2;
    return  max(query_depth(2 * node,  start,  mid,  l,  r),   query_depth(2 * node + 1,  mid + 1,  end,  l,  r));
}

void  dfs_sz(int u) {
    sz[u]  = 1;
    for  (int v : adj_rev[u]) {
        if  (!on_cycle[v]) {
            dfs_sz(v);
            sz[u]  += sz[v];
        }
    }
}

void  dfs_hld(int u) {
    tin[u]  = ++timer;
    
    int heavy  = -1,   max_s = -1;
    for  (int v : adj_rev[u]) {
        if  (!on_cycle[v]) {
            if  (sz[v] > max_s) {
                max_s  = sz[v];
                heavy  = v;
            }
        }
    }
    
    if  (heavy != -1) {
        top[heavy]  = top[u];
        depth[heavy]  = depth[u] + 1;
        parent[heavy]  = u;
        dfs_hld(heavy);
    }
    
    for  (int v : adj_rev[u]) {
        if  (!on_cycle[v] && v != heavy) {
            top[v]  = v;
            depth[v]  = depth[u] + 1;
            parent[v]  = u;
            dfs_hld(v);
        }
    }
    tout[u]  = timer;
}

int  get_nearest_active(int u) {
    int curr  = u;
    int best_depth  = 0;
    int best_node  = 0;
    
    while  (curr != 0 && !on_cycle[curr]) {
        int t  = top[curr];
        int d  = query_depth(1,  1,  N_val,  tin[t],  tin[curr]);
        if  (d > best_depth) {
            best_depth  = d;
        }
        curr  = parent[t];
    }
    
    return  -1;
}

int  tin_to_node[MAXN];

void  update_active_node(int u,   bool active) {
    update_depth(1,  1,  N_val,  tin[u],   active ? tin[u] : 0);
}

int  query_max_tin(int u) {
    int curr  = u;
    int max_tin  = 0;
    while  (curr != 0 && !on_cycle[curr]) {
        int t  = top[curr];
        int res  = query_depth(1,  1,  N_val,  tin[t],  tin[curr]);
        if  (res > max_tin)   max_tin = res;
        curr  = parent[t];
    }
    return  max_tin;
}

void  cycle_propagate(int c_id,   long long val,  int origin_idx) {
    if  (active_cycle_nodes[c_id].empty())   return;
    
    auto it  = active_cycle_nodes[c_id].lower_bound(origin_idx);
    int target_idx;
    if  (it == active_cycle_nodes[c_id].end()) {
        target_idx  = *active_cycle_nodes[c_id].begin();
    }  else {
        target_idx  = *it;
    }
    
    int target_node  = cycles[c_id][target_idx];
    
    ans[type[target_node]]  -= cnt[target_node];
    cnt[target_node]  += val;
    ans[type[target_node]]  += cnt[target_node];
}

int  main() {
    fast_io();
    
    string  FjString_str;
    if  (!(cin >> FjString_str))   return 0;
    N_val  = stoi(FjString_str);
    
    for  (int i = 1;  i <= N_val;  ++i) {
        cin  >> A[i];
        adj_rev[A[i]].push_back(i);
    }
    
    for  (int i = 1;  i <= N_val;  ++i) {
        if  (!vis[i]) {
            vector<int>  path;
            int curr  = i;
            while  (!vis[curr]) {
                vis[curr]  = 1;
                path.push_back(curr);
                curr  = A[curr];
            }
            
            bool found_cycle  = false;
            if  (vis[curr] == 1) {
                found_cycle  = true;
                cid_counter++;
                vector<int>&  cyc = cycles[cid_counter];
                
                bool in_cycle  = false;
                for  (int node : path) {
                    if  (node == curr)   in_cycle = true;
                    if  (in_cycle) {
                        on_cycle[node]  = cid_counter;
                        cyc.push_back(node);
                        cycle_pos[node]  = cyc.size() - 1;
                        root_of[node]  = node;
                        comp_id[node]  = cid_counter;
                    }
                }
                cycle_len[cid_counter]  = cyc.size();
            }
            
            int curr_fin  = i;
            while (vis[curr_fin] == 1) {
                vis[curr_fin]  = 2;
                curr_fin  = A[curr_fin];
            }
        }
    }
    
    for  (int c = 1;  c <= cid_counter;  ++c) {
        for  (int root : cycles[c]) {
            vector<int>  q;
            q.push_back(root);
            while (!q.empty()) {
                int u  = q.back();   q.pop_back();
                comp_id[u]  = c;
                root_of[u]  = root;
                for  (int v : adj_rev[u]) {
                    if  (!on_cycle[v]) {
                        q.push_back(v);
                    }
                }
            }
        }
    }
    
    timer  = 0;
    for  (int c = 1;  c <= cid_counter;  ++c) {
        for  (int root : cycles[c]) {
            top[root]  = root;
            depth[root]  = 0;
            parent[root]  = 0;
            sz[root]  = 1;
            for (int v : adj_rev[root]) {
                if (!on_cycle[v]) {
                    dfs_sz(v);
                    sz[root]  += sz[v];
                }
            }
            available[root]  = sz[root];
            
            for (int v : adj_rev[root]) {
                if (!on_cycle[v]) {
                    top[v]  = v;
                    depth[v]  = 1;
                    parent[v]  = root;
                    dfs_hld(v);
                }
            }
        }
        bit_cycle[c].resize(cycles[c].size() + 2,  0);
        for  (int j = 0;  j < cycles[c].size();  ++j) {
            update_cycle(c,  j + 1,  available[cycles[c][j]]);
        }
    }
    
    for (int i = 1;  i <= N_val;  ++i)   if (tin[i])   tin_to_node[tin[i]] = i;
    
    cin  >> M_val;
    
    while (M_val--) {
        int u;   char t;
        cin  >> u >> t;
        
        bool was_active  = (type[u] != 0);
        char old_type  = type[u];
        
        if  (was_active) {
            ans[old_type]  -= cnt[u];
            ans[t]  += cnt[u];
            type[u]  = t;
        }  else {
            type[u]  = t;
            
            if  (!on_cycle[u]) {
                long long caught_below  = query_tree(tout[u]) - query_tree(tin[u] - 1);
                cnt[u]  = sz[u] - caught_below;
                ans[t]  += cnt[u];
                
                update_tree(tin[u],  cnt[u]);
                
                update_active_node(u,  true);
                
                update_active_node(u,  false);
                int ancestor_tin  = query_max_tin(u);
                update_active_node(u,  true);
                
                if  (ancestor_tin != 0) {
                    int p  = tin_to_node[ancestor_tin];
                    ans[type[p]]  -= cnt[p];
                    update_tree(tin[p],  -cnt[p]);
                    
                    cnt[p]  -= cnt[u];
                    
                    update_tree(tin[p],  cnt[p]);
                    ans[type[p]]  += cnt[p];
                }  else {
                    int root  = root_of[u];
                    available[root]  -= cnt[u];
                    update_cycle(comp_id[root],  cycle_pos[root] + 1,  -cnt[u]);
                    cycle_propagate(comp_id[root],  -cnt[u],  cycle_pos[root]);
                }
            }  else {
                int cid  = comp_id[u];
                int idx  = cycle_pos[u];
                
                if  (active_cycle_nodes[cid].empty()) {
                    cnt[u]  = query_cycle_bit(cid,  cycle_len[cid]);
                    ans[t]  += cnt[u];
                    active_cycle_nodes[cid].insert(idx);
                }  else {
                    auto it  = active_cycle_nodes[cid].lower_bound(idx);
                    int s_idx,   p_idx;
                    
                    if  (it == active_cycle_nodes[cid].end()) {
                        s_idx  = *active_cycle_nodes[cid].begin();
                        p_idx  = *active_cycle_nodes[cid].rbegin();
                    }  else {
                        s_idx  = *it;
                        if  (it == active_cycle_nodes[cid].begin()) {
                            p_idx  = *active_cycle_nodes[cid].rbegin();
                        }  else {
                            p_idx  = *prev(it);
                        }
                    }
                    
                    int s_node  = cycles[cid][s_idx];
                    
                    long long val;
                    if  (p_idx < idx) {
                        val  = query_cycle_range(cid,  p_idx + 1,  idx);
                    }  else {
                        val  = query_cycle_range(cid,  p_idx + 1,  idx);
                    }
                    
                    ans[type[s_node]]  -= cnt[s_node];
                    cnt[s_node]  -= val;
                    ans[type[s_node]]  += cnt[s_node];
                    
                    cnt[u]  = val;
                    ans[t]  += cnt[u];
                    
                    active_cycle_nodes[cid].insert(idx);
                }
            }
        }
        
        cout  << ans['C'] << " " << ans['O'] << " " << ans['W'] << "\n";
    }
    
    return  0;
}
