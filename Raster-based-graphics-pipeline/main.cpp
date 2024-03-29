//
// Created by bishwajit on ১৭/৫/২১.
//

#include "point.h"
#include "matrix.h"
#include "helper.h"
#include "z_buffer.h"

#include <bits/stdc++.h>
using namespace std;


Point eye, look, up;
double fovY, aspectRatio, near, far;
Matrix cur = get_identity(SZ);

string scene, config;


void process_a_triangle(istream &in, ostream &out) {
    Point a, b, c;
    in >> a >> b >> c;
    a = cur * a;
    b = cur * b;
    c = cur * c;
    out << a << "\n" << b << "\n" << c << "\n";
    out << "\n";
}

void stage1(){
    stack<Matrix> st;
    ifstream in;
    in.open(scene);
    if (!in.is_open()) {
        cout << scene << " could not open" << endl;
        exit(1);
    }
    ofstream out;
    out.open("stage1.txt");

    if (!out.is_open()) {
        cout << "stage1" << " could not open" << endl;
        exit(1);
    }
    // read four lines
    in >> eye >> look >> up;
    in >> fovY >> aspectRatio >> near >> far;

    string cmd;
    while (in >> cmd){
        if (cmd == "triangle"){
            process_a_triangle(in, out);
        }
        else if (cmd == "translate"){
            Point tr;
            in >> tr;
            Matrix translate = create_translation_matrix(tr);
            cur = cur * translate;
        }
        else if (cmd == "scale"){
            Point sc;
            in >> sc;
            Matrix scale = create_scaling_matrix(sc);
            cur = cur * scale;
        }
        else if (cmd == "rotate"){
            double angle; // in degree
            Point axis;
            in >> angle >> axis;
            Matrix rotate = create_rotation_matrix(angle, axis);
            cur = cur * rotate;
        }
        else if (cmd == "push"){
            st.push(cur);
        }
        else if (cmd == "pop"){
            cur = st.top();
            st.pop();
        }
        else if (cmd == "end"){
            break;
        }
        else {
            cout << "Unknown Operation" << endl;
            break;
        }
    }
    in.close();
    out.close();
}


Matrix create_view_transformation_matrix(){
    Point l = look - eye;
    l = normalize(l);
    Point r = cross(l, up);
    r = normalize(r);
    Point u = cross(r, l);

    Matrix T = get_identity(SZ);

    T.mat[0][3] = -eye.x;
    T.mat[1][3] = -eye.y;
    T.mat[2][3] = -eye.z;

    Matrix R = get_identity(SZ);

    R.mat[0][0] = r.x;
    R.mat[1][0] = r.y;
    R.mat[2][0] = r.z;

    R.mat[0][1] = u.x;
    R.mat[1][1] = u.y;
    R.mat[2][1] = u.z;

    R.mat[0][2] = -l.x;
    R.mat[1][2] = -l.y;
    R.mat[2][2] = -l.z;

    return R * T;
}

void transform_entire_file(string inp_name, string out_name, Matrix &T){
    ifstream in;
    ofstream out;

    in.open(inp_name);
    out.open(out_name);

    Point a, b, c;

    while (in >> a >> b >> c){
        out << T * a << "\n";
        out << T * b << "\n";
        out << T * c << "\n";
        out << "\n";
    }
    in.close();
    out.close();
}

void stage2(){
    Matrix V = create_view_transformation_matrix();

    transform_entire_file("stage1.txt", "stage2.txt", V);
}

void stage3(){
    double fovX = fovY * aspectRatio;
    fovX = (PI/180.0) * fovX;
    fovY = (PI/180.0) * fovY;

    double t = near * tan(fovY/2);
    double r = near * tan(fovX/2);

    Matrix P = get_identity(SZ);

    P.mat[0][0] = near/r;
    P.mat[1][1] = near/t;
    P.mat[2][2] = -(far+near)/(far-near);
    P.mat[2][3] = -(2*far*near)/(far-near);
    P.mat[3][2] = -1;
    P.mat[3][3] = 0;

    transform_entire_file("stage2.txt",
                          "stage3.txt", P);

}

void stage4(){
    read_config(config);
    read_triangle("stage3.txt");
    z_buffer_algorithm();
}

int main(int argc, char* argv[]){
    srand(time(NULL));


    if (argc != 3){
        cout << "Provide scene.txt and config.txt files" << endl;
        exit(1);
    }

    scene = string(argv[1]);
    config = string(argv[2]);
    cout << scene << " " << config << endl;

    stage1();
    stage2();
    stage3();
    stage4();
    cout << "process ended" << endl;
}
