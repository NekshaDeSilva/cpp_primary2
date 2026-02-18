#include <iostream>
#include  <iomanip>
#include <cmath>
#include <vector>

using namespace std;
int main(){
    string np;
    cout << "Please enter a number" << endl;
    getline(cin, np);
    int n = stoi (np);
    if(!n == 0){
        n /= 2;
        
    }else if(!n == 0){
        n *=3;
        n += 1;

    }
    if (n ==1){
        return n;
    }

}
