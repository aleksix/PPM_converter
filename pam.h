#ifndef PPM_CONVERTER_PAM_H
#define PPM_CONVERTER_PAM_H

// Type of the PAM file
typedef enum {
	PBM = 1,
	PGM,
	PPM,
	PBM_BINARY,
	PGM_BINARY,
	PPM_BINARY
} pam_type;

// PAM file definition
typedef struct {
	pam_type type;
	unsigned int width;
	unsigned int height;
	// NOTE: Both of these sizes are not according to the standard
	// Standard for the ASCII formats requires
	//  maxval to be at most 65536 (short)
	// Meaning that maxval and image pixels need to be of type short
	// BUT no image I saw uses more than 255, so here's a byte.
	// Also, working with big-endian and little-endian
	//  would take a lot more time then I have
	unsigned char maxval;
	unsigned char **image;

	// Helper member to store the number of bytes-per-pixel(s)
	unsigned char bpp;
} pam;

// Initialize the pam structure and the image. Image is initialized to all 0
void init_pam(pam *out, const pam_type type, const unsigned char maxval,
		const unsigned int width, const unsigned int height);

// Initialize the image of the pam structure. Image is initialized to all 0
void init_pam_image(pam *out, const unsigned int width,
		const unsigned int height);

// Read a PAM file into a structure
int read_pam(const char *filename, pam *out);

// Try to copy one pam image into another with an offset.
// "force_copy" will create a new image identical in size to "in" to copy into
int copy(pam *in, pam *out, unsigned int force_copy, unsigned int offset_x,
		unsigned int offset_y);

// Save the structure into a file
// NOTE: the file will be overwritten if it already exists
int save_pam(const char *filename, const pam *out);

// Free the memory taken by the pam image
void free_pam(pam *out);

#endif //PPM_CONVERTER_PAM_H
