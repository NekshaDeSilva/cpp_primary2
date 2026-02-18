#include   <iostream>
#include      <vector>
#include <queue>
#include      <algorithm>
using namespace std;

const int INF = 1e9;

int n, f;
vector<int> adj;
vector<vector<int>> radj;
vector<int> starts;
vector<int> dist_farmer;
vector<int> deg;

vector<int> comp_id;
vector<int> cycle_entry;
vector<int> dist_to_cycle;
vector<int> cycle_pos;
vector<int> path_limit;

struct Component {
    vector<int> cycle_nodes;
    bool has_farmer = false;
    vector<bool> unsafe_mod;
    vector<int> best_sub;
};
vector<Component> components;

void bfs_farmers( ) {
    dist_farmer.assign(n + 1, INF);
    queue<int> q;
    for (int s : starts) {
        dist_farmer[s] = 0;
        q.push(s);
    }
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        int v = adj[u];
        if (dist_farmer[v] == INF) {
            dist_farmer[v] = dist_farmer[u] + 1;
            q.push(v);
        }
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    if (!(cin >> n >> f)) return 0;

    adj.resize(n + 1);
    radj.resize(n + 1);
    deg.assign(n + 1, 0);

    for (int i = 1; i <= n; i++) {
        cin >> adj[i];
        radj[adj[i]].push_back(i);
        deg[adj[i]]++;
    }

    starts.resize(f);
    for (int i = 0; i < f; i++) {
        cin >> starts[i];
    }

    bfs_farmers();

    queue<int> q;
    for (int i = 1; i <= n; i++) {
        if (deg[i] == 0) q.push(i);
    }
    vector<int> topo_order;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        topo_order.push_back(u);
        int v = adj[u];
        deg[v]--;
        if (deg[v] == 0) q.push(v);
    }

    comp_id.assign(n + 1, -1);
    cycle_pos.assign(n + 1, -1);
    vector<bool> visited(n + 1, false);
    int comp_count = 0;

    for (int i = 1; i <= n; i++) {
        if (deg[i] > 0 && !visited[i]) {
            Component comp;
            int curr = i;
            int idx = 0;
            while (!visited[curr]) {
                visited[curr] = true;
                comp.cycle_nodes.push_back(curr);
                comp_id[curr] = comp_count;
                cycle_pos[curr] = idx++;
                if (dist_farmer[curr] != INF) comp.has_farmer = true;
                curr = adj[curr];
            }
            components.push_back(comp);
            comp_count++;
        }
    }

    cycle_entry.resize(n + 1);
    dist_to_cycle.assign(n + 1, 0);

    for (int c = 0; c < comp_count; c++) {
        for (int u : components[c].cycle_nodes) {
            cycle_entry[u] = u;
        }
    }

    for (int i = topo_order.size() - 1; i >= 0; i--) {
        int u = topo_order[i];
        int v = adj[u];
        comp_id[u] = comp_id[v];
        cycle_entry[u] = cycle_entry[v];
        dist_to_cycle[u] = dist_to_cycle[v] + 1;
        if (dist_farmer[u] != INF) components[comp_id[u]].has_farmer = true;
    }

    path_limit.assign(n + 1, INF);

    for (int c = 0; c < comp_count; c++) {
        for (int u : components[c].cycle_nodes) {
            if (dist_farmer[u] != INF)
                path_limit[u] = dist_farmer[u] - 1;
        }
    }

    for (int i = topo_order.size() - 1; i >= 0; i--) {
        int u = topo_order[i];
        int v = adj[u];
        long long local = (dist_farmer[u] == INF) ? INF : (long long)dist_farmer[u] - 1;
        long long downstream = path_limit[v];
        if (downstream != INF) downstream--;
        path_limit[u] = min((long long)path_limit[u], min(local, downstream));
    }

    for (int c = 0; c < comp_count; c++) {
        if (!components[c].has_farmer) continue;
        int L = components[c].cycle_nodes.size();
        components[c].unsafe_mod.assign(L, false);
    }

    for (int s : starts) {
        int c = comp_id[s];
        int L = components[c].cycle_nodes.size();
        int e = cycle_entry[s];
        int d = dist_to_cycle[s];
        int p = cycle_pos[e];
        long long mod = (1LL * d - p) % L;
        if (mod < 0) mod += L;
        components[c].unsafe_mod[mod] = true;
    }

    for (int c = 0; c < comp_count; c++) {
        if (!components[c].has_farmer) continue;
        int L = components[c].cycle_nodes.size();
        components[c].best_sub.assign(L, -1);
        bool any_safe = false;
        for (bool b : components[c].unsafe_mod) if (!b) any_safe = true;
        if (!any_safe) continue;
        int last_safe = -1;
        for (int i = 0; i < 2 * L; i++) {
            int cur = i % L;
            if (!components[c].unsafe_mod[cur]) last_safe = i;
            if (i >= L) {
                if (last_safe != -1) {
                    components[c].best_sub[cur] = i - last_safe;
                }
            }
        }
    }

    for (int b = 1; b <= n; b++) {
        if (dist_farmer[b] == INF) {
            cout << -2 << "\n";
            continue;
        }
        long long limit = path_limit[b];
        if (limit < 0) {
            cout << -1 << "\n";
            continue;
        }
        int c = comp_id[b];
        if (!components[c].has_farmer) {
            cout << -2 << "\n";
            continue;
        }
        int L = components[c].cycle_nodes.size();
        int e = cycle_entry[b];
        int d = dist_to_cycle[b];
        int p = cycle_pos[e];
        long long target_mod = (1LL * d + limit - p) % L;
        if (target_mod < 0) target_mod += L;
        int sub = components[c].best_sub[target_mod];
        if (sub == -1) {
            cout << -1 << "\n";
        } else {
            if (limit - sub < 0) {
                cout << -1 << "\n";
            } else {
                cout << (limit - sub) << "\n";
            }
        }
    }
    return 0;
}
