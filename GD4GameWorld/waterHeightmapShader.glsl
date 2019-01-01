uniform sampler2D textureTwoFramesAgo;
uniform sampler2D textureOneFrameAgo;
uniform vec2 mousePosition;

const float textureSize = 500.0;
const float pixelSize = 1.0 / textureSize;

void main()
{
    // pixels position
    vec2 position = gl_TexCoord[0].st;

    vec4 finalColor = ((texture2D(textureOneFrameAgo, vec2(position.x - pixelSize, position.y)) +
                        texture2D(textureOneFrameAgo, vec2(position.x + pixelSize, position.y)) +
                        texture2D(textureOneFrameAgo, vec2(position.x, position.y + pixelSize)) +
                        texture2D(textureOneFrameAgo, vec2(position.x, position.y - pixelSize)) - 2.0) / 2) -
                       (texture2D(textureTwoFramesAgo, position) - 0.5);

    // damping
//    finalColor.rgb *= 1.9;  // <---- uncomment this for the "amplifiction" ie. to see the waves better
    finalColor.rgb += 0.5;

    // add new ripples
    if(mousePosition.x > 0.0)
    {
        if(distance(position, mousePosition) < pixelSize * 5.0)
        {
            finalColor = vec4(0.9, 0.9, 0.9, 1.0);
        }
    }

    gl_FragColor = finalColor;

}