#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <vector>
#include <cmath>

#include <iostream>
#include <chrono>

#define IWIDTH 800
#define IHEIGHT 600
#define ZOOMS 0.9

#define ITERATIONS 100
#define CX -1
#define CY 0
#define CW 4
#define CH 3

/*  #define ITERATIONS 5000
    #define CX -0.743643887037151
    #define CY 0.131825904205330
    #define CW 0.000000000051299
    #define CH 0.00000000003847425*/

template<typename T> T lerp( T, T, T );

void mandelbrot( sf::VertexArray&, long double, long double, long double, int, int, int, bool );

int main() {
    int width = IWIDTH, height = IHEIGHT;

    sf::RenderWindow window( sf::VideoMode( width, height ), "Mandelbrot Plotter" );
    sf::VertexArray graph( sf::Points, width * height );
    #pragma omp parallel for simd
    for ( int x = 0; x < width; x++ ) {
        for ( int y = 0; y < height; y++ )
            graph[x * height + y].position = sf::Vector2<float>( x, y );
    }

    long double zoom = 1;
    long double o_r = CX, o_i = CY;
    int iterations = ITERATIONS;

    bool left = false, right = false, up = false, down = false, zooming = false, unzooming = false, smooth = true, reload = true;

    sf::Clock frame_c;

    while ( window.isOpen() ) {
        sf::Event event;
        while ( window.pollEvent( event ) ) {
            if ( event.type == sf::Event::Closed )
                window.close();
            else {
                if ( event.type == sf::Event::Resized ) {
                    width = event.size.width;
                    height = event.size.height;
                    window.setView( sf::View( sf::FloatRect( 0, 0, width,  height ) ) );
                    graph = sf::VertexArray( sf::Points, width * height );
                    #pragma omp parallel for simd
                    for ( int x = 0; x < width; x++ ) {
                        for ( int y = 0; y < height; y++ )
                            graph[x * height + y].position = sf::Vector2<float>( x, y );
                    }
                    reload = true;
                }
                if ( event.type == sf::Event::KeyPressed ) {
                    if ( event.key.code == sf::Keyboard::Left )
                        left = true;
                    if ( event.key.code == sf::Keyboard::Up )
                        up = true;
                    if ( event.key.code == sf::Keyboard::Right )
                        right = true;
                    if ( event.key.code == sf::Keyboard::Down )
                        down = true;
                    if ( event.key.code == sf::Keyboard::Z )
                        zooming = true;
                    if ( event.key.code == sf::Keyboard::X )
                        unzooming = true;
                }
                if ( event.type == sf::Event::KeyReleased ) {
                    if ( event.key.code == sf::Keyboard::Left )
                        left = false;
                    if ( event.key.code == sf::Keyboard::Up )
                        up = false;
                    if ( event.key.code == sf::Keyboard::Right )
                        right = false;
                    if ( event.key.code == sf::Keyboard::Down )
                        down = false;
                    if ( event.key.code == sf::Keyboard::Z )
                        zooming = false;
                    if ( event.key.code == sf::Keyboard::X )
                        unzooming = false;
                    if ( event.key.code == sf::Keyboard::S ) {
                        smooth = !smooth;
                        reload = true;
                    }
                }
                if ( event.type == sf::Event::MouseWheelScrolled ) {
                    if ( event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel ) {
                        long double new_r, new_i, m_x = sf::Mouse::getPosition( window ).x, m_y = sf::Mouse::getPosition( window ).y;
                        if ( event.mouseWheelScroll.delta > 0 ) {
                            new_r = lerp( o_r, o_r + CW * ( ( ( long double )m_x / ( long double )width ) - 1.0 / 2.0 ) / zoom, ( long double )1.0 - ZOOMS );
                            new_i = lerp( o_i, o_i + CH * ( ( ( long double )m_y / ( long double )height ) - 1.0 / 2.0 ) / zoom, ( long double )1.0 - ZOOMS );
                            zoom /= ZOOMS;
                        }
                        else { /*if(zoom*ZOOMS >= 1.0)*/
                            new_r = lerp( o_r, o_r + CW * ( ( ( long double )m_x / ( long double )width ) - 1.0 / 2.0 ) / zoom, ( long double )1.0 - 1.0 / ZOOMS );
                            new_i = lerp( o_i, o_i + CH * ( ( ( long double )m_y / ( long double )height ) - 1.0 / 2.0 ) / zoom, ( long double )1.0 - 1.0 / ZOOMS );
                            zoom *= ZOOMS;
                        }
                        o_r = new_r;
                        o_i = new_i;
                        reload = true;
                    }
                }
            }
        }
        float delta_t = ( ( float )frame_c.restart().asMilliseconds() / 1000.0f );
        if ( zooming ) {
            zoom /= ZOOMS;
            reload = true;
        }
        if ( unzooming ) {
            //zoom = std::max(( long double )1.0, zoom*ZOOMS);
            zoom *= ZOOMS;
            reload = true;
        }
        if ( left ) {
            o_r -= ( CW / ( 2 * zoom ) ) * 1.0 / ZOOMS * delta_t;
            reload = true;
        }
        if ( up ) {
            o_i -= ( CH / ( 2 * zoom ) ) * 1.0 / ZOOMS * delta_t;
            reload = true;
        }
        if ( right ) {
            o_r += ( CW / ( 2 * zoom ) ) * 1.0 / ZOOMS * delta_t;
            reload = true;
        }
        if ( down ) {
            o_i += ( CH / ( 2 * zoom ) ) * 1.0 / ZOOMS * delta_t;
            reload = true;
        }
        if ( reload ) {
            auto start = std::chrono::high_resolution_clock::now();
            mandelbrot( graph, o_r, o_i, zoom, width, height, iterations, smooth );
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( stop - start );
            std::cout << "Time taken by function: " << duration.count() / 1000000.0 << " seconds" << std::endl;
            reload = false;
        }
        window.clear();
        window.draw( graph );
        window.display();
    }
    return EXIT_SUCCESS;
}

void mandelbrot( sf::VertexArray& graph, long double o_r, long double o_i, long double zoom, int width, int height, int iterations, bool smooth ) {
#define COLORS 16
    sf::Color colors[COLORS];
    colors[0] = sf::Color( 66, 30, 15 );
    colors[1] = sf::Color( 25, 7, 26 );
    colors[2] = sf::Color( 9, 1, 47 );
    colors[3] = sf::Color( 4, 4, 73 );
    colors[4] = sf::Color( 0, 7, 100 );
    colors[5] = sf::Color( 12, 44, 138 );
    colors[6] = sf::Color( 24, 82, 177 );
    colors[7] = sf::Color( 57, 125, 209 );
    colors[8] = sf::Color( 134, 181, 229 );
    colors[9] = sf::Color( 211, 236, 248 );
    colors[10] = sf::Color( 241, 233, 191 );
    colors[11] = sf::Color( 248, 201, 95 );
    colors[12] = sf::Color( 255, 170, 0 );
    colors[13] = sf::Color( 204, 128, 0 );
    colors[14] = sf::Color( 153, 87, 0 );
    colors[15] = sf::Color( 106, 52, 3 );
    #pragma omp parallel for schedule(dynamic)
    for ( int x = 0; x < width; x++ ) {
        for ( int y = 0; y < height; y++ ) {
            long double pos_r = o_r + CW * ( ( ( long double )x / ( long double )width ) - 1.0 / 2.0 ) / zoom,
                        pos_i = o_i + CH * ( ( ( long double )y / ( long double )height ) - 1.0 / 2.0 ) / zoom,
                        z_r = 0, z_i = 0, z2_r = z_r * z_r, z2_i = z_i * z_i;
            int i = 0;
            for ( ; z2_r + z2_i <= 4 && i < iterations; i++ ) {
                z_i = 2 * z_r * z_i + pos_i;
                z_r = z2_r - z2_i + pos_r;
                z2_r = z_r * z_r;
                z2_i = z_i * z_i;
            }
            if ( smooth ) {
                double mu = 1.0 + i - log( ( log( z2_r + z2_i ) / 2.0 )  / log( 2.0 ) ) / log( 2.0 );
                int color1 = ( int )mu, color2 = color1 + 1;
                double t2 = mu - color1, t1 = 1 - t2;
                graph[x * height + y].color = sf::Color( colors[color1 % COLORS].r * t1, colors[color1 % COLORS].g * t1, colors[color1 % COLORS].b * t1 ) + sf::Color( colors[color2 % COLORS].r * t2, colors[color2 % COLORS].g * t2, colors[color2 % COLORS].b * t2 );
            }
            else
                graph[x * height + y].color = colors[( int )( COLORS * ( ( double )i / iterations ) )];
        }
    }
}

template<typename T> T lerp( T a, T b, T v ) {
    return a + v * ( b - a );
}
