#define OLC_PGE_APPLICATION
#include <iostream>
#include <random>

#include "olcPixelGameEngine/olcPixelGameEngine.h"

constexpr float FLOAT_MIN = -5.0f;
constexpr float FLOAT_MAX = 5.0f;

std::random_device rd;
std::default_random_engine eng(rd());
std::uniform_real_distribution<float> distr(FLOAT_MIN, FLOAT_MAX);

void l(olc::vf2d &v, float max)
{
    const float mSq = v.mag();
    if (mSq > max * max)
    {
        v /= sqrt(mSq);
        v *= max;
    }
}

void m(olc::vf2d &v, float max)
{
    v.norm() *= max;
}

float dist(olc::vf2d &v1, olc::vf2d &v2)
{
    return sqrtf((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}
olc::vf2d direction = {0.0f, 0.0f};

class Flocking : public olc::PixelGameEngine
{
private:
    struct Particle
    {
        int id;
        olc::vf2d pos;
        olc::vf2d vel;
        olc::vf2d acc = {0.0f, 0.0f};
        float mass = 1.0f;
        float perception = 80.0f;

        float maxForce = 0.5f;
        float maxSpeed = 8.0f;

        bool canSee(olc::vf2d v, float r = 0.3f)
        {
            if (dist(pos, v) < perception)
            {
                olc::vf2d heading = vel.norm();

                direction = v - pos;
                direction = direction.norm();

                // https://www.youtube.com/watch?v=dC4-TSFv32Y
                // https://www.youtube.com/watch?v=BUwAcW_18Ws
                float product = direction.dot(v.norm());
                //float angle = acosf(product) * (180.0f / 3.14159);
                //float angle = acosf(std::min(1.0f, std::max(-1.0f, product))) + (3.14159 / 180.0f);

                //float angle = atan2f(heading.y, heading.x) - atan2f(direction.y, direction.x);
                // angle += 3.14159f / 2.0f;

                //std::cout << id << ": " << product << std::endl;

                return product > r;
            }

            return false;
        }

        olc::vf2d align(std::vector<Particle> &particles)
        {
            int total = 0;
            olc::vf2d steering = {0.0f, 0.0f};
            for (auto &p : particles)
            {
                if (id != p.id && canSee(p.pos))
                {
                    steering += p.vel;
                    total++;
                }
            }

            if (total > 0)
            {
                steering /= total;
                steering = steering.norm();
                steering *= maxSpeed;
                steering -= vel;

                return steering;
            }
            else
            {
                return olc::vf2d(0, 0);
            }
        }

        olc::vf2d cohesion(std::vector<Particle> &particles)
        {
            int total = 0;
            olc::vf2d steering = {0.0f, 0.0f};
            for (auto &p : particles)
            {
                if (id != p.id && canSee(p.pos, 0.8f))
                {
                    steering += p.pos;
                    total++;
                }
            }

            if (total > 0)
            {
                steering -= pos;
                steering = steering.norm();
                steering *= maxSpeed;
                steering -= vel;
            }

            return steering;
        }

        olc::vf2d separation(std::vector<Particle> &particles)
        {
            int total = 0;
            olc::vf2d steering = {0.0f, 0.0f};
            for (auto &p : particles)
            {
                if (id != p.id && canSee(p.pos, 0.8f))
                {
                    float fDistance = dist(pos, p.pos);
                    olc::vf2d diff = pos - p.pos;
                    diff = diff.norm();
                    diff /= (fDistance * fDistance);
                    steering += diff;
                    total++;
                }
            }

            if (total > 0)
            {
                steering /= total;
            }

            if (steering.mag() > 0)
            {
                steering = steering.norm();
                steering *= maxSpeed;
                steering -= vel;
            }

            return steering;
        }

        void applyForce(olc::vf2d force)
        {
            acc += force / mass;
        }

        void update(void)
        {
            vel += acc;
            l(vel, maxSpeed);
            pos += vel;
            acc = olc::vf2d(0.0f, 0.0f);
        }
    };

    olc::vf2d rotate(olc::vf2d &p, float r)
    {
        float rx = p.x * cosf(r) - p.y * sinf(r);
        float ry = p.x * sinf(r) + p.y * cosf(r);

        return olc::vf2d(rx, ry);
    }

    std::vector<Particle> particles;
    bool paused = false;
    bool debugLines = false;

    float aliW = 0.5f;
    float sepW = 1.0f;
    float cohW = 1.0f;

public:
    Flocking()
    {
        sAppName = "Flocking";
    }

    void populate()
    {
        particles.clear();
        for (int i = 0; i < 200; i++)
        {
            Particle p;
            p.id = i;
            p.pos = {((float)rand() / (float)RAND_MAX) * ScreenWidth(), ((float)rand() / (float)RAND_MAX) * ScreenHeight()};
            p.vel = {distr(eng), distr(eng)};
            particles.emplace_back(p);
        }
    }

    bool OnUserCreate() override
    {
        populate();
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        if (GetKey(olc::Key::A).bHeld && GetKey(olc::Key::UP).bPressed)
        {
            aliW += 0.5f;
        }
        else if (GetKey(olc::Key::A).bHeld && GetKey(olc::Key::DOWN).bPressed)
        {
            aliW -= 0.5f;
        }

        if (GetKey(olc::Key::C).bHeld && GetKey(olc::Key::UP).bPressed)
        {
            cohW += 0.5f;
        }
        else if (GetKey(olc::Key::C).bHeld && GetKey(olc::Key::DOWN).bPressed)
        {
            cohW -= 0.5f;
        }

        if (GetKey(olc::Key::S).bHeld && GetKey(olc::Key::UP).bPressed)
        {
            sepW += 0.5f;
        }
        else if (GetKey(olc::Key::S).bHeld && GetKey(olc::Key::DOWN).bPressed)
        {
            sepW -= 0.5f;
        }

        if (GetKey(olc::Key::D).bReleased)
        {
            debugLines = !debugLines;
        }

        if (GetKey(olc::Key::R).bReleased)
        {
            populate();
        }

        for (auto &p : particles)
        {
            olc::vf2d ali = p.align(particles) * fElapsedTime;
            olc::vf2d sep = p.separation(particles) * fElapsedTime;
            olc::vf2d coh = p.cohesion(particles) * fElapsedTime;

            // weight each force
            ali *= aliW;
            sep *= sepW;
            coh *= cohW;

            p.applyForce(ali);
            p.applyForce(sep);
            p.applyForce(coh);

            if (p.pos.x > ScreenWidth())
            {
                p.pos.x = 0;
            }
            else if (p.pos.x < 0)
            {
                p.pos.x = ScreenWidth();
            }

            if (p.pos.y > ScreenHeight())
            {
                p.pos.y = 0;
            }
            else if (p.pos.y < 0)
            {
                p.pos.y = ScreenHeight();
            }

            p.update();
        }

        Clear(olc::BLACK);
        DrawString(ScreenWidth() - 400, 20, "Alignment: " + std::to_string(aliW), olc::WHITE, 2.0f);
        DrawString(ScreenWidth() - 400, 50, "Seperation: " + std::to_string(sepW), olc::WHITE, 2.0f);
        DrawString(ScreenWidth() - 400, 80, "Cohesion: " + std::to_string(cohW), olc::WHITE, 2.0f);

        for (auto &p : particles)
        {
            float r = 2.5f;

            olc::vf2d h = p.vel.norm();
            float angle = atan2f(h.y, h.x) + 3.14159f / 2.0f;

            olc::vf2d p1 = olc::vf2d(0.0f, -5.0f);
            olc::vf2d p2 = olc::vf2d(-2.5f, 2.5f);
            olc::vf2d p3 = olc::vf2d(2.5f, 2.5f);

            p1 = rotate(p1, angle);
            p2 = rotate(p2, angle);
            p3 = rotate(p3, angle);

            p1 *= r;
            p2 *= r;
            p3 *= r;

            p1.x += p.pos.x;
            p1.y += p.pos.y;

            p2.x += p.pos.x;
            p2.y += p.pos.y;

            p3.x += p.pos.x;
            p3.y += p.pos.y;

            FillTriangle(p1, p2, p3, olc::Pixel(255, 255, 255, 150));

            if (debugLines)
            {
                if (p.id == 0)
                {
                    DrawLine(p.pos, p.pos + (direction * 50.0f), olc::BLUE);
                }
            }
        }

        return !GetKey(olc::Key::ESCAPE).bPressed;
    }
};

int main(int, char **)
{
    Flocking flocking;
    if (flocking.Construct(1900, 1600, 1, 1))
        flocking.Start();

    return 0;
}
