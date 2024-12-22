#version 330 compatibility

uniform float   uKa, uKd, uKs;	// coefficients of each type of lighting
uniform float   uShininess;		// specular exponent

in  vec2  vST;	// texture coords
in  vec3  vN;	// normal vector
in  vec3  vL;	// vector from point to light
in  vec3  vE;	// vector from point to eye

const float EYE		= 0.91; // both eyes S coordinate
const float EYE1T	= 0.36; // first eyes T coordinate
const float EYE2T	= 0.64; // second eyes T coordinate
const float R		= 0.02;	// radius of salmon eye
const vec3 SALMONCOLOR		= vec3( 0.98, 0.50, 0.45 );		// "salmon" (r,g,b) color
const vec3 ACCENTCOLOR		= vec3( 0.85, 0.52, 0.46 );		// "accent" (r,g,b) color
const vec3 EYECOLOR			= vec3( 0., 1., 0. );			// color to make the eye
const vec3 SPECULARCOLOR	= vec3( 1., 1., 1. );

void
main( )
{
	vec3 myColor = SALMONCOLOR;
	vec3 accentColor = ACCENTCOLOR;

    // Check distance for first eye
    float ds1 = vST.s - EYE;
    float dt1 = vST.t - EYE1T;
    
    // Check distance for second eye
    float ds2 = vST.s - EYE;
    float dt2 = vST.t - EYE2T;

    // Color fragment if it is around either eye
    if( (ds1 * ds1 + dt1 * dt1 / 4) <= (R * R) || (ds2 * ds2 + dt2 * dt2 / 4) <= (R * R) )
    {
        myColor = EYECOLOR;
    }

	// now do the per-fragment lighting:
	vec3 Normal    = normalize(vN);
	vec3 Light     = normalize(vL);
	vec3 Eye       = normalize(vE);

	vec3 ambient = uKa * myColor;

	// only do diffuse if the light can see the point
	float d = max( dot(Normal,Light), 0. );       
	vec3 diffuse = uKd * d * myColor;

	float s = 0.;

	// only do specular if the light can see the point
	if( d > 0. )	          
	{
		vec3 ref = normalize(  reflect( -Light, Normal )  );
		float cosphi = dot( Eye, ref );
		if( cosphi > 0. )
			s = pow( max( cosphi, 0. ), uShininess );
	}
	vec3 specular = uKs * s * SPECULARCOLOR.rgb;
	gl_FragColor = vec4( ambient + diffuse + specular,  1. );
}
