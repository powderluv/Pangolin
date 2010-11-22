#ifndef PANGOLIN_GL_H
#define PANGOLIN_GL_H

#include <map>
#include "pangolin.h"

#include <GL/gl.h>

#ifdef HAVE_GLUT
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

#define GLUT_KEY_ESCAPE 27
#define GLUT_KEY_TAB 9

namespace pangolin
{

  //! @brief Give this OpenGL context a name or switch contexts
  //! This is required to initialise Pangolin for use with an
  //! externally defined OpenGL context. You needn't call it
  //! if you have used CreateGlutWindowAndBind() to create a GLUT
  //! window
  void BindToContext(std::string name);

  //! @brief Returns true if user has requested to close OpenGL window.
  bool ShouldQuit();

  namespace process
  {
    //! @brief Tell pangolin to process input to drive display
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Keyboard( unsigned char key, int x, int y);

    //! @brief Tell pangolin base window size has changed
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Resize(int width, int height);

    void Mouse( int button, int state, int x, int y);

    void MouseMotion( int x, int y);
  }

#ifdef HAVE_GLUT
  namespace glut
  {
    //! @brief Create GLUT window and bind Pangolin to it.
    //! All GLUT initialisation is taken care of. This prevents you
    //! from needing to call BindToContext() and TakeGlutCallbacks().
    void CreateWindowAndBind(std::string window_title, int w = 640, int h = 480 );

    //! @brief Allow Pangolin to take GLUT callbacks for its own uses
    //! Not needed if you instantiated a window through CreateWindowAndBind().
    void TakeCallbacks();
  }
#endif

  //! @brief Unit for measuring quantities
  enum Unit {
    Fraction,
    Pixel
  };

  //! @brief Defines typed quantity
  //! Constructors distinguised by whole pixels, or floating
  //! fraction in interval [0,1]
  struct Attach {
    Attach() : unit(Fraction), p(0) {}
    Attach(int p) : unit(Pixel), p(p) {}
    Attach(GLfloat p) : unit(Fraction), p(p) {}
    Attach(GLdouble p) : unit(Fraction), p(p) {}
    Unit unit;
    GLfloat p;
  };

  //! @brief Encapsulates OpenGl Viewport
  struct Viewport
  {
    Viewport() {}
    Viewport(GLint l,GLint b,GLint w,GLint h);
    void Activate() const;
    bool Contains(int x, int y) const;
    GLint r() const;
    GLint t() const;
    GLfloat aspect() const;
    GLint l,b,w,h;
  };

  //! @brief Capture OpenGL matrix types in enum to typing
  enum OpenGlStack {
    GlProjection = GL_PROJECTION,
    GlModelView = GL_MODELVIEW,
    GlTexture = GL_TEXTURE
  };

  struct CameraSpec {
    GLdouble forward[3];
    GLdouble up[3];
    GLdouble right[3];
    GLdouble img_up[2];
    GLdouble img_right[2];
  };

  const static CameraSpec CameraSpecOpenGl = {{0,0,-1},{0,1,0},{1,0,0},{0,1},{1,0}};
  const static CameraSpec CameraSpecYDownZForward = {{0,0,1},{0,-1,0},{1,0,0},{0,-1},{1,0}};

  //! @brief Object representing OpenGl Matrix
  struct OpenGlMatrixSpec {
    // Load matrix on to OpenGl stack
    void Load() const;

    // Specify which stack this refers to
    OpenGlStack type;

    // Column major Internal buffer
    GLdouble m[16];
  };

  //! @brief Object representing attached OpenGl Matrices / transforms
  //! Only stores what is attached, not entire OpenGl state (which would
  //! be horribly slow). Applying state is efficient.
  struct OpenGlRenderState
  {
    void Apply() const;
    std::map<OpenGlStack,OpenGlMatrixSpec> stacks;
  };

  // Forward declaration
  struct Handler;

  //! @brief A Display manages the location and resizing of an OpenGl viewport.
  struct Display
  {
    Display() : handler(0) {}

    //! Activate Displays viewport for drawing within this area
    void Activate() const;

    void Activate(const OpenGlRenderState& state ) const;

    //! Given the specification of Display, compute viewport
    void RecomputeViewport(const Viewport& parent);

    // Desired width / height aspect (0 if dynamic)
    float aspect;

    // Bounds to fit display within
    Attach top, left, right, bottom;

    // Cached absolute viewport (recomputed on resize)
    Viewport v;

    // Access sub-displays by name
    Display*& operator[](std::string name);

    // Input event handler (if any)
    Handler* handler;

    // Map for sub-displays (if any)
    std::map<std::string,Display*> displays;
  };

  //! @brief Input Handler base class with virtual methods which recurse
  //! into sub-displays
  struct Handler
  {
    virtual void Keyboard(Display&, unsigned char key, int x, int y);
    virtual void Mouse(Display&, int button, int state, int x, int y);
    virtual void MouseMotion(Display&, int x, int y);
  };
  static Handler StaticHandler;

  struct Handler3D : Handler
  {
    Handler3D(OpenGlRenderState& cam_state)
      : cam_state(&cam_state), tf(0.01), cameraspec(CameraSpecOpenGl), move_mode(0) {};

    void SetOpenGlCamera();
    void Mouse(Display&, int button, int state, int x, int y);
    void MouseMotion(Display&, int x, int y);

    OpenGlRenderState* cam_state;
    float tf;
    CameraSpec cameraspec;
    int move_mode;
    float last_pos[2];
  };

  Display* AddDisplay(std::string name, Attach top, Attach left, Attach bottom, Attach right, bool keep_aspect = false );
  Display* AddDisplay(std::string name, Attach top, Attach left, Attach bottom, Attach right, float aspect );
  Display*& GetDisplay(std::string name);

  OpenGlMatrixSpec ProjectionMatrix(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );
  OpenGlMatrixSpec IdentityMatrix(OpenGlStack type);

}

inline pangolin::Viewport::Viewport(GLint l,GLint b,GLint w,GLint h) : l(l),b(b),w(w),h(h) {};
inline GLint pangolin::Viewport::r() const { return l+w;}
inline GLint pangolin::Viewport::t() const { return b+h;}
inline GLfloat pangolin::Viewport::aspect() const { return (GLfloat)w / (GLfloat)h; }


#endif // PANGOLIN_GL_H
