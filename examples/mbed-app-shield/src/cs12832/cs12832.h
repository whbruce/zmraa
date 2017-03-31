#pragma once

struct _Bitmap {
    int xSize;
    int ySize;
    int Byte_in_Line;
    char* data;
};

typedef struct _Bitmap Bitmap;

/** Draw mode
  * NORMAl
  * XOR set pixel by xor the screen
  */
enum {NORMAL,XOR};


void cs12832_init(int bus, int lcd_a0_pin, int lcd_reset_pin);
void cs12832_fill(unsigned char value);

int c12832_width();

/** Get the height of the screen in pixel
 *
 * @returns height of screen in pixel
 *
 */
int c12832_height();

/** Draw a pixel at x,y black or white
 *
 * @param x horizontal position
 * @param y vertical position
 * @param colour ,1 set pixel ,0 erase pixel
 */
void c12832_pixel(int x, int y,int colour);

/** draw a circle
  *
  * @param x0,y0 center
  * @param r radius
  * @param colour ,1 set pixel ,0 erase pixel
  *
  */
void c12832_circle(int x, int y, int r, int colour);

/** draw a filled circle
 *
 * @param x0,y0 center
 * @param r radius
 * @param color ,1 set pixel ,0 erase pixel
 *
 * use circle with different radius,
 * can miss some pixel
 */
void c12832_fillcircle(int x, int y, int r, int colour);

/** draw a 1 pixel line
  *
  * @param x0,y0 start point
  * @param x1,y1 stop point
  * @param color ,1 set pixel ,0 erase pixel
  *
  */
void c12832_line(int x0, int y0, int x1, int y1, int colour);

/** draw a rect
*
* @param x0,y0 top left corner
* @param x1,y1 down right corner
* @param color 1 set pixel ,0 erase pixel
*                                                   *
*/
void c12832_rect(int x0, int y0, int x1, int y1, int colour);

/** draw a filled rect
  *
  * @param x0,y0 top left corner
  * @param x1,y1 down right corner
  * @param color 1 set pixel ,0 erase pixel
  *
  */
void c12832_fillrect(int x0, int y0, int x1, int y1, int colour);

/** copy display buffer to lcd
  *
  */

void c12832_copy_to_lcd(void);

/** set the orienation of the screen
  *
  */


void c12832_set_contrast(unsigned int o);

/** read the contrast level
  *
  */
unsigned int c12832_get_contrast(void);


/** invert the screen
  *
  * @param o = 0 normal, 1 invert
  */
void c12832_invert(unsigned int o);

/** clear the screen
   *
   */
void c12832_cls(void);

/** set the drawing mode
  *
  * @param mode NORMAl or XOR
  */

void c12832_setmode(int mode);

int c12832_columns(void);

/** calculate the max number of columns
 *
 * @returns max column
 * depends on actual font size
 *
 */
int c12832_rows(void);

/** put a char on the screen
 *
 * @param value char to print
 * @returns printed char
 *
 */
int c12832_putc(char value);

/** put a string on the screen
 *
 * @param string to print
 *
 */
void c12832_puts(char* str);

/** draw a character on given position out of the active font to the LCD
 *
 * @param x x-position of char (top left)
 * @param y y-position
 * @param c char to print
 *
 */
void c12832_character(int x, int y, int c);

/** setup cursor position
 *
 * @param x x-position (top left)
 * @param y y-position
 */
void c12832_locate(int x, int y);

/** setup auto update of screen
  *
  * @param up 1 = on , 0 = off
  * if switched off the program has to call copy_to_lcd()
  * to update screen from framebuffer
  */
void c12832_set_auto_up(unsigned int up);

/** get status of the auto update function
  *
  *  @returns if auto update is on
  */
unsigned int c12832_get_auto_up(void);
