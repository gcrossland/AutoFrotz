/* quetzal.c  - Saving and restoring of Quetzal files.
 *	Written by Martin Frost <mdf@doc.ic.ac.uk>
 *
 * This file is part of Frotz.
 *
 * Frotz is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Frotz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <string.h>
#include "../common/frotz.h"

using autofrotz::vmlink::ZbyteReader;
using autofrotz::vmlink::ZbyteWriter;

typedef unsigned long zlong;

vmlocal extern zword frames[];

unsigned int random_statesize (void);
void random_savestate (unsigned char *buffer);
void random_restorestate (unsigned char *buffer);

/*
 * ID types.
 */

#define makeid(a,b,c,d) ((zlong) (((a)<<24) | ((b)<<16) | ((c)<<8) | (d)))

#define ID_FORM makeid ('F','O','R','M')
#define ID_IFZS makeid ('I','F','Z','S')
#define ID_IFhd makeid ('I','F','h','d')
#define ID_CMem makeid ('C','M','e','m')
#define ID_Stks makeid ('S','t','k','s')
#define ID_ANNO makeid ('A','N','N','O')
#define ID_FRng makeid ('F','R','n','g')

/*
 * Various parsing states within restoration.
 */

#define GOT_HEADER	0x01
#define GOT_STACK	0x02
#define GOT_MEMORY	0x04
#define GOT_NONE	0x00
#define GOT_ALL		0x07
#define GOT_ERROR	0x80

/*
 * Interface used to write the files.
 */

template<typename _T> static long tell (_T &f)
{
    return f.tell();
}

template<typename _T> static bool seekto (_T &f, long offset)
{
    return f.seekTo(offset);
}

template<typename _T> static bool seekby (_T &f, long offset)
{
    return f.seekBy(offset);
}

static int get_c(ZbyteReader &f)
{
    return f.getByte();
}

static int put_c(int c, ZbyteWriter &f)
{
    zbyte b = c;
    f.setByte(b);
    return b;
}

static bool write_byte(ZbyteWriter &f, int c)
{
    f.setByte(c);
    return true;
}

#define write_bytx write_byte

static bool write_word(ZbyteWriter &f, int c)
{
    f.setWord(c);
    return true;
}

#define write_long(fp,l) \
    (write_bytx (fp, (l) >> 24) && write_bytx (fp, (l) >> 16) && \
     write_bytx (fp, (l) >>  8) && write_bytx (fp, (l)))
#define write_chnk(fp,id,len) \
    (write_long (fp, (id))      && write_long (fp, (len)))
#define write_run(fp,run) \
    (write_byte (fp, 0)         && write_byte (fp, (run)))

/* Read one word from file; return TRUE if OK. */
static bool read_word (ZbyteReader &f, zword *result)
{
    iu32 w;
    if ((w = f.getWord()) == EOF) return false;

    *result = w;
    return true;
}

/* Read one long from file; return TRUE if OK. */
static bool read_long (ZbyteReader &f, zlong *result)
{
    iu32 h, l;
    if ((h = f.getWord()) == EOF) return false;
    if ((l = f.getWord()) == EOF) return false;

    *result = (static_cast<zlong>(h) << 16) | static_cast<zlong>(l);
    return true;
}

/*
 * Restore a saved game using Quetzal format. Return 2 if OK, 0 if an error
 * occurred before any damage was done, -1 on a fatal error.
 */

zword auto_restore_quetzal (ZbyteReader &svf, ZbyteReader &stf)
{
    zlong ifzslen, currlen, tmpl;
    zlong pc;
    zword i, tmpw;
    zword fatal = 0;	/* Set to -1 when errors must be fatal. */
    zbyte skip, progress = GOT_NONE;
    int x, y;

    /* Check it's really an `IFZS' file. */
    if (!read_long (svf, &tmpl)
	|| !read_long (svf, &ifzslen)
	|| !read_long (svf, &currlen))				return 0;
    if (tmpl != ID_FORM || currlen != ID_IFZS)
    {
	print_string ("This is not a saved game file!\n");
	return 0;
    }
    if ((ifzslen & 1) || ifzslen<4) /* Sanity checks. */	return 0;
    ifzslen -= 4;

    /* Read each chunk and process it. */
    while (ifzslen > 0)
    {
	/* Read chunk header. */
	if (ifzslen < 8) /* Couldn't contain a chunk. */	return 0;
	if (!read_long (svf, &tmpl)
	    || !read_long (svf, &currlen))			return 0;
	ifzslen -= 8;	/* Reduce remaining by size of header. */

	/* Handle chunk body. */
	if (ifzslen < currlen) /* Chunk goes past EOF?! */	return 0;
	skip = currlen & 1;
	ifzslen -= currlen + (zlong) skip;

	switch (tmpl)
	{
	    /* `IFhd' header chunk; must be first in file. */
	    case ID_IFhd:
		if (progress & GOT_HEADER)
		{
		    print_string ("Save file has two IFZS chunks!\n");
		    return fatal;
		}
		progress |= GOT_HEADER;
		if (currlen < 13
		    || !read_word (svf, &tmpw))			return fatal;
		if (tmpw != h_release)
		    progress = GOT_ERROR;

		for (i=H_SERIAL; i<H_SERIAL+6; ++i)
		{
		    if ((x = get_c (svf)) == EOF)		return fatal;
		    if (x != zmp[i])
			progress = GOT_ERROR;
		}

		if (!read_word (svf, &tmpw))			return fatal;
		if (tmpw != h_checksum)
		    progress = GOT_ERROR;

		if (progress & GOT_ERROR)
		{
		    print_string ("File was not saved from this story!\n");
		    return fatal;
		}
		if ((x = get_c (svf)) == EOF)			return fatal;
		pc = (zlong) x << 16;
		if ((x = get_c (svf)) == EOF)			return fatal;
		pc |= (zlong) x << 8;
		if ((x = get_c (svf)) == EOF)			return fatal;
		pc |= (zlong) x;
		fatal = -1;	/* Setting PC means errors must be fatal. */
		SET_PC (pc);

		for (i=13; i<currlen; ++i)
		    (void) get_c (svf);	/* Skip rest of chunk. */
		break;
	    /* `Stks' stacks chunk; restoring this is quite complex. ;) */
	    case ID_Stks:
		if (progress & GOT_STACK)
		{
		    print_string ("File contains two stack chunks!\n");
		    break;
		}
		progress |= GOT_STACK;

		fatal = -1;	/* Setting SP means errors must be fatal. */
		sp = stack + STACK_SIZE;

		/*
		 * All versions other than V6 may use evaluation stack outside
		 * any function context. As a result a faked function context
		 * will be present in the file here. We skip this context, but
		 * load the associated stack onto the stack proper...
		 */
		if (h_version != V6)
		{
		    if (currlen < 8)				return fatal;
		    for (i=0; i<6; ++i)
			if (get_c (svf) != 0)			return fatal;
		    if (!read_word (svf, &tmpw))		return fatal;
		    if (tmpw > STACK_SIZE)
		    {
			print_string ("Save-file has too much stack (and I can't cope).\n");
			return fatal;
		    }
		    currlen -= 8;
		    if (currlen < tmpw*2)			return fatal;
		    for (i=0; i<tmpw; ++i)
			if (!read_word (svf, --sp))		return fatal;
		    currlen -= tmpw*2;
		}

		/* We now proceed to load the main block of stack frames. */
		for (fp = stack+STACK_SIZE, frame_count = 0;
		     currlen > 0;
		     currlen -= 8, ++frame_count)
		{
		    if (currlen < 8)				return fatal;
		    if (sp - stack < 4)	/* No space for frame. */
		    {
			print_string ("Save-file has too much stack (and I can't cope).\n");
			return fatal;
		    }

		    /* Read PC, procedure flag and formal param count. */
		    if (!read_long (svf, &tmpl))		return fatal;
		    y = (int) (tmpl & 0x0F);	/* Number of formals. */
		    tmpw = y << 8;

		    /* Read result variable. */
		    if ((x = get_c (svf)) == EOF)		return fatal;

		    /* Check the procedure flag... */
		    if (tmpl & 0x10)
		    {
			tmpw |= 0x1000;	/* It's a procedure. */
			tmpl >>= 8;	/* Shift to get PC value. */
		    }
		    else
		    {
			/* Functions have type 0, so no need to or anything. */
			tmpl >>= 8;	/* Shift to get PC value. */
			--tmpl;		/* Point at result byte. */
			/* Sanity check on result variable... */
			if (zmp[tmpl] != (zbyte) x)
			{
			    print_string ("Save-file has wrong variable number on stack (possibly wrong game version?)\n");
			    return fatal;
			}
		    }
		    *--sp = (zword) (tmpl >> 9);	/* High part of PC */
		    *--sp = (zword) (tmpl & 0x1FF);	/* Low part of PC */
		    *--sp = (zword) (fp - stack - 1);	/* FP */

		    /* Read and process argument mask. */
		    if ((x = get_c (svf)) == EOF)		return fatal;
		    ++x;	/* Should now be a power of 2 */
		    for (i=0; i<8; ++i)
			if (x & (1<<i))
			    break;
		    if (x ^ (1<<i))	/* Not a power of 2 */
		    {
			print_string ("Save-file uses incomplete argument lists (which I can't handle)\n");
			return fatal;
		    }
		    *--sp = tmpw | i;
		    fp = sp;	/* FP for next frame. */

		    /* Read amount of eval stack used. */
		    if (!read_word (svf, &tmpw))		return fatal;

		    tmpw += y;	/* Amount of stack + number of locals. */
		    if (sp - stack <= tmpw)
		    {
			print_string ("Save-file has too much stack (and I can't cope).\n");
			return fatal;
		    }
		    if (currlen < tmpw*2)			return fatal;
		    for (i=0; i<tmpw; ++i)
			if (!read_word (svf, --sp))		return fatal;
		    currlen -= tmpw*2;
		}
		/* End of `Stks' processing... */
		break;
	    /* `FRng' header chunk; restore random number generator state. */
	    case ID_FRng:
		if (currlen != random_statesize())
		{
		    print_string ("'FRng' chunk size isn't compatible with the random state size.\n");
		    return fatal;
		}
		else
		{
		   unsigned char buffer[random_statesize()];

		   for (i = 0; i < random_statesize(); i++)
		   {
			if ((x = get_c (svf)) == EOF)		return fatal;
			*(buffer + i) = (unsigned char) x;
		   }

		   random_restorestate(buffer);
		}
		break;
	    /* Any more special chunk types must go in HERE or ABOVE. */
	    /* `CMem' compressed memory chunk; uncompress it. */
	    case ID_CMem:
		if (!(progress & GOT_MEMORY))	/* Don't complain if two. */
		{
		    if (!seekto (stf, 0))			return fatal;
		    i=0;	/* Bytes written to data area. */
		    for (; currlen > 0; --currlen)
		    {
			if ((x = get_c (svf)) == EOF)		return fatal;
			if (x == 0)	/* Start run. */
			{
			    /* Check for bogus run. */
			    if (currlen < 2)
			    {
				print_string ("File contains bogus `CMem' chunk.\n");
				for (; currlen > 0; --currlen)
				    (void) get_c (svf);	/* Skip rest. */
				currlen = 1;
				i = 0xFFFF;
				break; /* Keep going; may be a `UMem' too. */
			    }
			    /* Copy story file to memory during the run. */
			    --currlen;
			    if ((x = get_c (svf)) == EOF)	return fatal;
			    for (; x >= 0 && i<h_dynamic_size; --x, ++i)
				if ((y = get_c (stf)) == EOF)	return fatal;
				else
				    zmp[i] = (zbyte) y;
			}
			else	/* Not a run. */
			{
			    if ((y = get_c (stf)) == EOF)	return fatal;
			    zmp[i] = (zbyte) (x ^ y);
			    ++i;
			}
			/* Make sure we don't load too much. */
			if (i > h_dynamic_size)
			{
			    print_string ("warning: `CMem' chunk too long!\n");
			    for (; currlen > 1; --currlen)
				(void) get_c (svf);	/* Skip rest. */
			    break;	/* Keep going; there may be a `UMem' too. */
			}
		    }
		    /* If chunk is short, assume a run. */
		    for (; i<h_dynamic_size; ++i)
			if ((y = get_c (stf)) == EOF)		return fatal;
			else
			    zmp[i] = (zbyte) y;
		    if (currlen == 0)
			progress |= GOT_MEMORY;	/* Only if succeeded. */
		    break;
		}
		/* Fall right thru (to default) if already GOT_MEMORY */
	    /* Unrecognised chunk type; skip it. */
	    default:
		/* Skip chunk. */
		if (!seekby (svf, currlen))			return fatal;
		break;
	}
	if (skip)
	    (void) get_c (svf);	/* Skip pad byte. */
    }

    /*
     * We've reached the end of the file. For the restoration to have been a
     * success, we must have had one of each of the required chunks.
     */
    if (!(progress & GOT_HEADER))
	print_string ("error: no valid header (`IFhd') chunk in file.\n");
    if (!(progress & GOT_STACK))
	print_string ("error: no valid stack (`Stks') chunk in file.\n");
    if (!(progress & GOT_MEMORY))
	print_string ("error: no valid memory (`CMem' or `UMem') chunk in file.\n");

    return (progress == GOT_ALL ? 2 : fatal);
}

/*
 * Save a game using Quetzal format. Return 1 if OK, 0 if failed.
 */

zword auto_save_quetzal (ZbyteWriter &svf, ZbyteReader &stf)
{
    zlong ifzslen = 0, cmemlen = 0, stkslen = 0, frnglen = 0;
    zlong pc;
    zword i, j, n;
    zword nvars, nargs, nstk, *p;
    zbyte var;
    long cmempos, stkspos;
    int c;

    /* Write `IFZS' header. */
    if (!write_chnk (svf, ID_FORM, 0))			return 0;
    if (!write_long (svf, ID_IFZS))			return 0;

    /* Write `IFhd' chunk. */
    GET_PC (pc);
    if (!write_chnk (svf, ID_IFhd, 13))			return 0;
    if (!write_word (svf, h_release))			return 0;
    for (i=H_SERIAL; i<H_SERIAL+6; ++i)
	if (!write_byte (svf, zmp[i]))			return 0;
    if (!write_word (svf, h_checksum))			return 0;
    if (!write_long (svf, pc << 8)) /* Includes pad. */	return 0;

    /* Write `CMem' chunk. */
    if ((cmempos = tell (svf)) < 0)			return 0;
    if (!write_chnk (svf, ID_CMem, 0))			return 0;
    if (!seekto (stf, 0))				return 0;
    /* j holds current run length. */
    for (i=0, j=0, cmemlen=0; i < h_dynamic_size; ++i)
    {
	if ((c = get_c (stf)) == EOF)			return 0;
	c ^= (int) zmp[i];
	if (c == 0)
	    ++j;	/* It's a run of equal bytes. */
	else
	{
	    /* Write out any run there may be. */
	    if (j > 0)
	    {
		for (; j > 0x100; j -= 0x100)
		{
		    if (!write_run (svf, 0xFF))		return 0;
		    cmemlen += 2;
		}
		if (!write_run (svf, j-1))		return 0;
		cmemlen += 2;
		j = 0;
	    }
	    /* Any runs are now written. Write this (nonzero) byte. */
	    if (!write_byte (svf, (zbyte) c))		return 0;
	    ++cmemlen;
	}
    }
    /*
     * Reached end of dynamic memory. We ignore any unwritten run there may be
     * at this point.
     */
    if (cmemlen & 1)	/* Chunk length must be even. */
	if (!write_byte (svf, 0))			return 0;

    /* Write `Stks' chunk. You are not expected to understand this. ;) */
    if ((stkspos = tell (svf)) < 0)			return 0;
    if (!write_chnk (svf, ID_Stks, 0))			return 0;

    /*
     * We construct a list of frame indices, most recent first, in `frames'.
     * These indices are the offsets into the `stack' array of the word before
     * the first word pushed in each frame.
     */
    frames[0] = sp - stack;	/* The frame we'd get by doing a call now. */
    for (i = fp - stack + 4, n=0; i < STACK_SIZE+4; i = stack[i-3] + 5)
	frames[++n] = i;

    /*
     * All versions other than V6 can use evaluation stack outside a function
     * context. We write a faked stack frame (most fields zero) to cater for
     * this.
     */
    if (h_version != V6)
    {
	for (i=0; i<6; ++i)
	    if (!write_byte (svf, 0))			return 0;
	nstk = STACK_SIZE - frames[n];
	if (!write_word (svf, nstk))			return 0;
	for (j=STACK_SIZE-1; j >= frames[n]; --j)
	    if (!write_word (svf, stack[j]))		return 0;
	stkslen = 8 + 2*nstk;
    }

    /* Write out the rest of the stack frames. */
    for (i=n; i>0; --i)
    {
	p = stack + frames[i] - 4;	/* Points to call frame. */
	nvars = (p[0] & 0x0F00) >> 8;
	nargs =  p[0] & 0x00FF;
	nstk  =  frames[i] - frames[i-1] - nvars - 4;
	pc    =  ((zlong) p[3] << 9) | p[2];

	switch (p[0] & 0xF000)	/* Check type of call. */
	{
	    case 0x0000:	/* Function. */
		var = zmp[pc];
		pc = ((pc + 1) << 8) | nvars;
		break;
	    case 0x1000:	/* Procedure. */
		var = 0;
		pc = (pc << 8) | 0x10 | nvars;	/* Set procedure flag. */
		break;
	    /* case 0x2000: */
	    default:
		runtime_error (ERR_SAVE_IN_INTER);
		return 0;
	}
	if (nargs != 0)
	    nargs = (1 << nargs) - 1;	/* Make args into bitmap. */

	/* Write the main part of the frame... */
	if (!write_long (svf, pc)
	    || !write_byte (svf, var)
	    || !write_byte (svf, nargs)
	    || !write_word (svf, nstk))			return 0;

	/* Write the variables and eval stack. */
	for (j=0, ++p; j<nvars+nstk; ++j, --p)
	    if (!write_word (svf, *p))			return 0;

	/* Calculate length written thus far. */
	stkslen += 8 + 2 * (nvars + nstk);
    }

    /* Write `FRng' chunk. */
    frnglen = random_statesize();
    {
	unsigned char buffer[frnglen];

	random_savestate(buffer);

	if (!write_chnk (svf, ID_FRng, frnglen))	return 0;
	for (i = 0; i < frnglen; i++)
	    if (!write_byte (svf, *(buffer + i)))	return 0;
	if (frnglen & 1) /* Chunk length must be even. */
	    if (!write_byte (svf, 0))			return 0;
    }

    /* Fill in variable chunk lengths. */
    ifzslen = 4*8 + 4 + 14 + cmemlen + stkslen + frnglen;
    if (cmemlen & 1)
	++ifzslen;
    if (frnglen & 1)
	++ifzslen;
    if (!seekto (svf,         4))			return 0;
    if (!write_long (svf, ifzslen))			return 0;
    if (!seekto (svf, cmempos+4))			return 0;
    if (!write_long (svf, cmemlen))			return 0;
    if (!seekto (svf, stkspos+4))			return 0;
    if (!write_long (svf, stkslen))			return 0;

    /* After all that, still nothing went wrong! */
    return 1;
}

zword auto_restore_quetzal ()
{
    if (!autofrotz::vmlink::vmLink->hasRestoreState ())
    {
        print_string ("There is no State to restore from.\n");
        return 0;
    }

    ZbyteReader stateReader = autofrotz::vmlink::vmLink->createRestoreStateReader();
    ZbyteReader storyReader = autofrotz::vmlink::vmLink->createInitialDynamicMemoryReader();
    zword r = auto_restore_quetzal (stateReader, storyReader);
    if (r == 2)
    {
        autofrotz::vmlink::vmLink->restoreSucceeded ();
    }
    return r;
}

zword auto_save_quetzal ()
{
    if (!autofrotz::vmlink::vmLink->hasSaveState ())
    {
        print_string ("There is no State to save to.\n");
        return 0;
    }

    ZbyteWriter stateWriter = autofrotz::vmlink::vmLink->createSaveStateWriter();
    ZbyteReader storyReader = autofrotz::vmlink::vmLink->createInitialDynamicMemoryReader();
    zword r = auto_save_quetzal (stateWriter, storyReader);
    if (r == 1)
    {
        autofrotz::vmlink::vmLink->saveSucceeded ();
    }
    return r;
}
