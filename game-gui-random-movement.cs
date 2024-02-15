class RandomMovement {
    private Vec2 min;
    private Vec2 max;
    private Vec2 yRotChange;
    private float lerpSpeed = 0.05f;  // 0.01f - 0.1f

    void awake()
    {
        newPosition = transform.position;
        newRotation = transform.rotation;
    }

    void update()
    {
        transform.position = lerp(transform.position, newPosition, deltaTime * lerpSpeed);
        transform.rotation = slerp(transform.rortation, newRotation, deltaTime * lerpSpeed);

        if(distance(transform.position, newPosition) < 1f)
        {
            getNewPosition();
        }
    }

    void getNewPosition()
    {
        float xPos = random(min.x, max.x);
        float zPos = random(min.y, max.y);
        newRotation = fromEuler(0, random(rotationRange), 0);
        newPosition = (xPos, 0, zPos);
    }
}

