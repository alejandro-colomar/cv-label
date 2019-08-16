/******************************************************************************
 *	Copyright (C) 2018	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include "label/label.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <opencv2/imgproc/types_c.h>
#include <zbar.h>

#include <libalx/base/compiler/size.h>
#include <libalx/base/errno/error.h>
#include <libalx/base/stdio/printf/sbprintf.h>
#include <libalx/base/stdlib/maximum.h>
#include <libalx/extra/cv/alx.h>
#include <libalx/extra/cv/core.h>
#include <libalx/extra/cv/highgui.h>
#include <libalx/extra/cv/imgproc.h>
#include <libalx/extra/ocr/ocr.h>
#include <libalx/extra/zbar/zbar.h>


/******************************************************************************
 ******* macros ***************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* enum / struct / union ************************************************
 ******************************************************************************/


/******************************************************************************
 ******* static functions (prototypes) ****************************************
 ******************************************************************************/
static
int	init_cv		(img_s **img_1, img_s **img_2, rect_rot_s **rect_rot);
static
void	deinit_cv	(img_s *img_1, img_s *img_2, rect_rot_s *rect_rot);
static
int	proc_label_steps(const char *restrict fname);
static
int	find_label	(const img_s *restrict img,
			 rect_rot_s *restrict rect_rot);
static
int	align_label	(img_s *restrict img,
			 const rect_rot_s *restrict rect_rot);
static
int	find_cerdo	(const img_s *restrict img,
			 const rect_rot_s *restrict rect_rot);
static
int	read_barcode	(const img_s *restrict img,
			 char bcode[static restrict BUFSIZ]);
static
int	read_price	(const img_s *restrict img,
			 const rect_rot_s *restrict rect_rot,
			 char price[static restrict BUFSIZ]);
static
int	chk_bcode_prod	(const char bcode[static restrict BUFSIZ]);
static
int	chk_bcode_price	(const char bcode[static restrict BUFSIZ],
			 const char price[static restrict BUFSIZ]);
static
void	report		(const char bcode[static restrict BUFSIZ],
			 const char price[static restrict BUFSIZ]);


/******************************************************************************
 ******* global functions *****************************************************
 ******************************************************************************/
int	proc_label	(const char *restrict fname)
{
	clock_t	time_0;
	clock_t	time_1;
	double	time_tot;

	time_0 = clock();
	if (proc_label_steps(fname))
		return	-1;
	time_1 = clock();

	time_tot = ((double) time_1 - time_0) / CLOCKS_PER_SEC;
	printf("Total time:	%5.3lf s;\n", time_tot);

	return	0;
}


/******************************************************************************
 ******* static functions (definitions) ***************************************
 ******************************************************************************/
static
int	init_cv		(img_s **img_1, img_s **img_2, rect_rot_s **rect_rot)
{

	if (alx_cv_alloc_img(img_1))
		return	-1;
	if (alx_cv_init_img(*img_1, 1, 1))
		goto out_free_1;
	if (alx_cv_alloc_img(img_2))
		goto out_deinit_1;
	if (alx_cv_init_img(*img_2, 1, 1))
		goto out_free_2;
	if (alx_cv_alloc_rect_rot(rect_rot))
		goto out_deinit_2;
	return	0;

out_deinit_2:
	alx_cv_deinit_img(img_2);
out_free_2:
	alx_cv_free_img(img_2);
out_deinit_1:
	alx_cv_deinit_img(img_1);
out_free_1:
	alx_cv_free_img(img_1);
	return	-1;
}

static
void	deinit_cv	(img_s *img_1, img_s *img_2, rect_rot_s *rect_rot)
{

	alx_cv_free_rect_rot(rect_rot);
	alx_cv_deinit_img(img_2);
	alx_cv_free_img(img_2);
	alx_cv_deinit_img(img_1);
	alx_cv_free_img(img_1);
}

static
int	proc_label_steps(const char *restrict fname)
{
	img_s		*img;
	img_s		*img_tmp;
	rect_rot_s	*rect_rot;
	char		bcode[BUFSIZ];
	char		price[BUFSIZ];
	int		status;

	status	= -1;
	if (init_cv(&img, &img_tmp, &rect_rot))
		return	-1;
	if (alx_cv_imread(img, fname))
		goto err;

	if (find_label(/*const*/img, rect_rot)) {
		fprintf(stderr, "Label not found");
		goto err;
	}
	if (alx_cv_component(img, ALX_CV_CMP_GREEN)) {
		fprintf(stderr, "Couldn't extract green component");
		goto err;
	}
	alx_cv_clone(img_tmp, img);
	if (align_label(img_tmp, /*const*/rect_rot)) {
		fprintf(stderr, "Couldn't align label");
		goto err;
	}
	if (find_cerdo(/*const*/img_tmp, /*const*/rect_rot)) {
		fprintf(stderr, "\"Cerdo\" not found");
		goto err;
	}
	if (read_barcode(/*const*/img, bcode)) {
		fprintf(stderr, "Barcode not found");
		goto err;
	}
	if (read_price(/*const*/img_tmp, /*const*/rect_rot, price)) {
		fprintf(stderr, "Price not found");
		goto err;
	}
	if (chk_bcode_prod(/*const*/bcode)) {
		fprintf(stderr, "Product doesn't match in barcode");
		goto err;
	}
	if (chk_bcode_price(/*const*/bcode, price)) {
		fprintf(stderr, "Price doesn't match in barcode");
		goto err;
	}

	report(bcode, price);
	status	= 0;

err:
	deinit_cv(img, img_tmp, rect_rot);
	return	status;
}

static
int	find_label	(const img_s *restrict img,
			 rect_rot_s *restrict rect_rot)
{
	img_s		*img_tmp;
	conts_s		*conts;
	const cont_s	*cont;
	int		status;

	status	= -1;

	if (alx_cv_alloc_img(&img_tmp))
		return	-1;
	if (alx_cv_init_img(img_tmp, 1, 1))
		goto out_free_tmp;
	if (alx_cv_alloc_conts(conts))
		goto out_deinit_tmp;
	alx_cv_init_conts(conts);

	alx_cv_clone(img_tmp, img);
	if (alx_cv_component(img_tmp, ALX_CV_CMP_BLUE))
		goto err;
	if (alx_cv_smooth(img_tmp, ALX_CV_SMOOTH_MEDIAN, 7))
		goto err;
	alx_cv_invert(img_tmp);
	if (alx_cv_smooth(img_tmp, ALX_CV_SMOOTH_MEAN, 21))
		goto err;
	if (alx_cv_threshold(img_tmp, CV_THRESH_BINARY_INV, 2))
		goto err;
	if (alx_cv_dilate_erode(img_tmp, 100))
		goto err;
	if (alx_cv_contours(img_tmp, conts))
		goto err;
	if (alx_cv_contours_size(conts) != 1)
		goto err;
	if (alx_cv_contours_contour(&cont, conts, 0))
		goto err;
	alx_cv_min_area_rect(rect_rot, cont);

	status	= 0;
err:
	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
out_deinit_tmp:
	alx_cv_deinit_img(img_tmp);
out_free_tmp:
	alx_cv_free_img(img_tmp);
	return	status;
}

static
int	align_label	(img_s *restrict img,
			 const rect_rot_s *restrict rect_rot)
{

	if (alx_cv_rotate_2rect(img, rect_rot))
		return	-1;
	return	0;
}

static
int	find_cerdo	(const img_s *restrict img,
			 const rect_rot_s *restrict rect_rot)
{
	img_s		*img_tmp;
	rect_s		*rect;
	ptrdiff_t	rectrot_ctr_x, rectrot_ctr_y, rectrot_w, rectrot_h;
	ptrdiff_t	lbl_x, lbl_y, lbl_w, lbl_h;
	void		*imgdata;
	ptrdiff_t	imgdata_w, imgdata_h, B_per_pix, B_per_line;
	char		text[BUFSIZ];
	int		status;

	status	= -1;

	if (alx_cv_alloc_img(&img_tmp))
		return	-1;
	if (alx_cv_init_img(img_tmp, 1, 1))
		goto out_free_tmp;
	if (alx_cv_alloc_rect(&rect))
		goto out_deinit_tmp;

	alx_cv_clone(img_tmp, img);
	alx_cv_extract_rect_rot(rect_rot, &rectrot_ctr_x, &rectrot_ctr_y,
					&rectrot_w, &rectrot_h);
	lbl_x	= rectrot_ctr_x - (1.05 * rectrot_w / 2);
	lbl_y	= rectrot_ctr_y - (1.47 * rectrot_h / 2);
	lbl_w	= rectrot_w / 2;
	lbl_h	= rectrot_h * 0.20;
	if (lbl_x < 0)
		lbl_x = 0;
	if (lbl_y < 0)
		lbl_y = 0;
	if (alx_cv_init_rect(rect, lbl_x, lbl_y, lbl_w, lbl_h))
		goto err;
	alx_cv_roi_set(img_tmp, rect);
	if (alx_cv_threshold(img_tmp, CV_THRESH_BINARY, ALX_CV_THR_OTSU))
		goto err;
	if (alx_cv_erode(img_tmp, 1))
		goto err;
	alx_cv_extract_imgdata(img_tmp, &imgdata, NULL, NULL,
					&imgdata_w, &imgdata_h,
					&B_per_pix, &B_per_line);
	if (alx_ocr_read(ARRAY_SIZE(text), text, imgdata, imgdata_w, imgdata_h,
					B_per_pix, B_per_line,
					ALX_OCR_LANG_SPA, ALX_OCR_CONF_NONE))
		goto err;

	if (strncmp(text, "Cerdo", strlen("Cerdo")))
		status	= 1;
	else
		status	= 0;
err:
	alx_cv_free_rect(rect);
out_deinit_tmp:
	alx_cv_deinit_img(img_tmp);
out_free_tmp:
	alx_cv_free_img(img_tmp);
	return	status;
}

static
int	read_barcode	(const img_s *restrict img,
			 char bcode[static restrict BUFSIZ])
{
	img_s		*img_tmp;
	void		*imgdata;
	ptrdiff_t	rows, cols;
	int		status;

	status	= -1;

	if (alx_cv_alloc_img(&img_tmp))
		return	-1;
	if (alx_cv_init_img(img_tmp, 1, 1))
		goto out_free_tmp;

	alx_cv_clone(img_tmp, img);
	alx_cv_extract_imgdata(img_tmp, &imgdata, &rows, &cols, NULL, NULL,
								NULL, NULL);
	if (alx_zbar_read(BUFSIZ, bcode, NULL, imgdata, rows, cols, ZBAR_EAN13))
		goto err;

	status	= 0;
err:
	alx_cv_deinit_img(img_tmp);
out_free_tmp:
	alx_cv_free_img(img_tmp);
	return	status;
}

static
int	read_price	(const img_s *restrict img,
			 const rect_rot_s *restrict rect_rot,
			 char price[static restrict BUFSIZ])
{
	img_s		*img_tmp;
	rect_s		*rect;
	ptrdiff_t	rectrot_ctr_x, rectrot_ctr_y, rectrot_w, rectrot_h;
	ptrdiff_t	prc_x, prc_y, prc_w, prc_h;
	void		*imgdata;
	ptrdiff_t	imgdata_w, imgdata_h, B_per_pix, B_per_line;
	int		status;

	status	= -1;

	if (alx_cv_alloc_img(&img_tmp))
		return	-1;
	if (alx_cv_init_img(img_tmp, 1, 1))
		goto out_free_tmp;
	if (alx_cv_alloc_rect(&rect))
		goto out_deinit_tmp;

	alx_cv_clone(img_tmp, img);
	alx_cv_extract_rect_rot(rect_rot, &rectrot_ctr_x, &rectrot_ctr_y,
					&rectrot_w, &rectrot_h);
	prc_x	= rectrot_ctr_x - (0.33 * rectrot_w / 2);
	prc_y	= rectrot_ctr_y - (0.64 * rectrot_h / 2);
	prc_w	= rectrot_w * 0.225;
	prc_h	= rectrot_h * 0.15;
	if (prc_x < 0)
		prc_x = 0;
	if (prc_y < 0)
		prc_y = 0;
	if (alx_cv_init_rect(rect, prc_x, prc_y, prc_w, prc_h))
		goto err;
	alx_cv_roi_set(img_tmp, rect);
	if (alx_cv_smooth(img_tmp, ALX_CV_SMOOTH_MEAN, 3))
		goto err;
	if (alx_cv_threshold(img_tmp, CV_THRESH_BINARY, ALX_CV_THR_OTSU))
		goto err;
	if (alx_cv_dilate_erode(img_tmp, 1))
		goto err;
	if (alx_cv_threshold(img_tmp, CV_THRESH_BINARY, 1))
		goto err;
	alx_cv_extract_imgdata(img_tmp, &imgdata, NULL, NULL,
					&imgdata_w, &imgdata_h,
					&B_per_pix, &B_per_line);
	if (alx_ocr_read(BUFSIZ, price, imgdata, imgdata_w, imgdata_h,
					B_per_pix, B_per_line,
					ALX_OCR_LANG_DIGITS,
					ALX_OCR_CONF_PRICE_EUR))
		goto err;

	status	= 0;
err:
	alx_cv_free_rect(rect);
out_deinit_tmp:
	alx_cv_deinit_img(img_tmp);
out_free_tmp:
	alx_cv_free_img(img_tmp);
	return	status;
}

static
int	chk_bcode_prod	(const char bcode[static restrict BUFSIZ])
{

	if (strncmp(bcode, "2301703", strlen("2301703")))
		return	1;
	return	0;
}

static
int	chk_bcode_price	(const char bcode[static restrict BUFSIZ],
			 const char price[static restrict BUFSIZ])
{
	char	bcode_price[BUFSIZ];

	if (bcode[8] != '0') {
		if (alx_sbprintf(bcode_price, NULL, "%c%c.%c%c",
							bcode[8], bcode[9],
							bcode[10], bcode[11]))
			return	-1;
	} else {
		if (alx_sbprintf(bcode_price, NULL, "%c.%c%c",
							bcode[9],
							bcode[10], bcode[11]))
			return	-1;
	}

	if (strncmp(price, bcode_price, strlen(bcode_price)))
		return	1;
	return	0;
}

static
void	report		(const char bcode[static restrict BUFSIZ],
			 const char price[static restrict BUFSIZ])
{

	printf("Code:	%s\n", bcode);
	printf("Price:	%5s EUR\n", price);
}


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
