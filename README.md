# **Cpp5 Framework**
A small implementation in C++ (mostly C) of the Processing and p5.js frameworks using OpenGL and no additional dependencies.

### **Build**:
Install [Visual Studio Community 2019](https://visualstudio.microsoft.com/).
Run shell.bat to setup the Visual Studio build tools.
    Note: Change shell.bat if you installed Visual Studio on a different location.
Run build_all.bat to build all examples.

## **Framework API**
## Vectors
        An Euclidean vector is an entity that has both magnitude and direction.
            
        You can use vectors to represent different things:
        
        Position vector
        A position can be a vector representing the difference between position and origin.
        Therefore you can think of it as the distance (difference) between two points. 
            magnitude -> the distance between two points, the length of the vector
            direction -> the angle that the vector is pointing in
        
        
        Velocity vector
        magnitude -> represent speed, the faster the speed the longer the length
        direction -> the angle (heading) in which the object is moving

        This declares a vector that are 10 units in the x-direction and 5 units in the y-direction,
        you can also think of it as an x- and y-coordinate:
            v2 vec = v2(10.f, 5.f);

    Vector initializiation
    v2i vec = v2i(1, 1);
    v2 vec = { 1.f, 1.f };
    v2 vec = v2(2.f, 2.f);
    v3 vec = v3(2.f, 2.f, 2.f);
    v4 vec = v4(2.f, 2.f, 2.f,, 2.f);
    v2 vec{ 1.f, 1.f };
        
    Operator overloads
    vec1 + vec2;
    vec1 - vec2;
    vec1 * 0.5f;
    vec1 / 2;
    -vec1;
    
    Methods
    vec.length()
    vec.heading();
    vec.setLength(10);
    vec.setAngle(3.f);
    vec.limit(2);
    vec.normalize();
    
    Functions
    v2Hadamard(vec1, vec2);
    v2DotProduct(vec1, vec2);
    v2Add(vec1, vec2);
	v2Sub(vec1, vec2);
    v2Mul(vec1, 2.0f);
    dist(p1, p2);
        
    v2 result = v2Add(vec1, vec2);
       Vector addition, c-style.
    
	v2 result = v2Sub(vec1, vec2);
        Vector subtraction, c-style
    
    v2 result = v2Mul(vec, 2.0f);
        Vector multiplication with a scalar, c-style.
    
    v2 result = vec1 + vec2;
        Vector addition using operator overloading.
    
    v2 result = vec1 - vec2;
        Vector subtraction using operator overloading.
        
    result = vec * 0.5f;
        Vector multiplication with a scalar using operator overloading.
        
    result = vec / 2;
        Vector division with a scalar using operator overloading.
        
    result = -vec;
        Vector negation, vector equals it's negative.

    vec.length()
        Returns the length (magnitude) of the vector using The Pythagorean Theorem, 
        square root of x^2 + y^2 -> same as sqrt(dotProduct(*this)).
        
    vec.heading();
        Returns the angle of rotation, the heading (direction), of the vector.
    
    vec.setLength(10);
        Set length of vector.
    
    vec.setAngle(3.f);
        Set the angle of the vector.
    
    vec.limit(2);
        Limit the magnitude of the vector.
    
    vec.normalize();
        Calculate a unit vector. Normalizing a vector makes its length equal to 1. To normalize a vector - divide each component by its length.
    
    v2 result = v2Hadamard(vec1, vec2);
        The hadamard product - element-wise product of two vectors which return a new vector.
    
    f32 result = v2DotProduct(vec1, vec2);
        Calculate the dot(inner) product which gives us the angle between two vectors, returns a scalar.
        The dot product consists of multiplying each element of the A vector with its counterpart from vector
        B and taking the sum of each product.
      
    f32 result = dist(p1, p2);
        Using the Pythagorean Theorem to calculate the distance between 2 points, returns a float, same as
        calculating a vectors length.
   

## **Examples**
    shapes
        Demonstration of basic shapes in 3D that can be drawn using the framework.
    
    vectors
        Example of vector math. Shows how to add, subtract, scale, normalize and calculate the magnitude
        (length) of vectors.

    lissajous_curves
	    A couple of examples of Lissajous curves.

    snow
        Falling snowfakes.

    stars2d
        2D starfield.

    stars3d
        3D starfield.

