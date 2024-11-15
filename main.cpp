#include<iostream>
#include<unistd.h>
#include<math.h>
#include<vector>

// screen
const int RESOLUTION_X = 100;
const int RESOLUTION_Y = 40;
const int SYMBOLS_SIZE = 70;
long long curr_frame = 0;
const std::string SYMBOLS = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,.^`'. ";
std::string screen = "";
void drawOnScreen(int x, int y, char symbol){
    screen[y*(RESOLUTION_X + 1/*new line symbol*/)+x] = symbol;
}
void clearScreen(){
    std::string clear = "";
    for(int i=0; i<50; i++){
        clear += "\n";
    } 
    std::cout<<clear;
}
void resetScreen(){
    screen = "";
    for(int y=0; y<RESOLUTION_Y; y++){
        for(int x=0; x<RESOLUTION_X; x++){
            screen +=  " ";
        }
        screen += "\n";
    }
}

// 3d engine
struct vec3{
    float x, y, z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    // Addition
    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }
    // Subtraction
    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }
    // Scalar Multiplication
    vec3 operator*(float scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }
    // Scalar Division
    vec3 operator/(float scalar) const {
        if (scalar != 0)
            return vec3(x / scalar, y / scalar, z / scalar);
        else
            throw std::runtime_error("Division by zero");
    }
    // Dot Product
    float operator*(const vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    // Equality
    bool operator==(const vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    // Inequality
    bool operator!=(const vec3& other) const {
        return !(*this == other);
    }
    // Output Stream
    friend std::ostream& operator<<(std::ostream& os, const vec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};
float len(const vec3& v){
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}
vec3 normalize(const vec3& v){
    float l =  sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if(l == 0){
        return vec3(0, 0, 0);
    }
    return v/l;
}
vec3 cross(const vec3& a, const vec3& b) {
    return vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}
struct Triangle{
    vec3 p1, p2, p3;
    Triangle(vec3 p1_, vec3 p2_, vec3 p3_) : p1(p1_), p2(p2_), p3(p3_) {}
};
struct Ray{
    vec3 pos, dir;
    Ray(vec3 p, vec3 d) : pos(p), dir(d) {}
};
struct Camera{
    vec3 pos;
    float focusDist;
    int res_x, res_y;
    bool isActive;
    Camera(vec3 p, int rx, int ry, float fd, bool active=false) : pos(p), res_x(rx), res_y(ry), focusDist(fd), isActive(active) {}
    float checkTriangle(Ray r, Triangle tr){
        vec3 edgeAB = tr.p2 - tr.p1;
        vec3 edgeAC = tr.p3 - tr.p1;
        vec3 N = cross(edgeAB, edgeAC); // normal of the plane
        float D = -(N*tr.p1); // Ax + By + Cz + D = 0
        float t = -(N*r.pos + D) / (N*r.dir);
        vec3 point = r.pos + r.dir*t;
        // if point in triangle
        N = normalize(N);
        // if(point == tr.p1 || point == tr.p2 || point == tr.p3){
        //     return t;
        // }
        bool ac_right = len(normalize(cross(point-tr.p1, tr.p3-tr.p1)) - N) < 0.01;
        bool ab_left = len(normalize(cross(tr.p2-tr.p1, point-tr.p1)) - N) < 0.01;
        bool bc_left = len(normalize(cross(tr.p3-tr.p2, point-tr.p2)) - N) < 0.01;
        if(!(ac_right && ab_left && bc_left)){
            return -1;
        }
        return t;
    }
    float castRay(Ray r, std::vector<Triangle> &triangles){
        float nas = -1;
        for(auto &tr : triangles){
            float dist = checkTriangle(r, tr);
            if(dist < 0){
                continue;
            }
            if(nas > dist || nas == -1){
                nas = dist;
            }
        }
        return nas;
    }
    void castRays(std::vector<Triangle> &triangles){
        if(!isActive){
            return;
        }
        for(int y=0; y<res_y; y++){
            for(int x=0; x<res_x; x++){
                vec3 screenPoint(pos.x-res_x/2.0+x, pos.y-res_y/2.0+y, pos.z+focusDist);
                float d = castRay(Ray(pos, screenPoint-pos), triangles);
                if(d < 0){
                    continue;
                }
                float brightness = d*10;
                drawOnScreen(x, y, SYMBOLS[(int)brightness]);
            }
        }
    }
};

// ============================================================================================
// cls && g++ main.cpp -o main.exe && main.exe

Camera camera(vec3(), RESOLUTION_X, RESOLUTION_Y, 50.0, true);
std::vector<Triangle> triangles;
std::pair<int, int> point;
void init(){
    point.first = 0;
    point.second = 10;
    triangles.push_back(Triangle(vec3(-0.5, -0.5, 50), vec3(3.5, 2.5, 50), vec3(4.0, -2.0, 50)));
} 
void update(){
    if(curr_frame%10 == 0){
        point.first++;
    }
    if(triangles[0].p1.z > 5){
        triangles[0].p1.z--;
    }else if(triangles[0].p2.z > 5){
        triangles[0].p2.z--;
    }else if(triangles[0].p3.z > 5){
        triangles[0].p3.z--;
    }

    camera.castRays(triangles);
    drawOnScreen(point.first, point.second, '@');
}
int main(){
    init();
    while(true){
        clearScreen();
        resetScreen();
        update();

        std::string line;
        for(int i=0; i<RESOLUTION_X; i++){
            line += "_";
        }
        std::cout<<line<<"\n";
        std::cout<<screen;
        std::cout<<line<<"\n";
        
        usleep(3000);
        curr_frame++;
        // break;
    }
    return 0;
}