/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxtexman.c,v 1.3 2000/12/08 21:34:20 alanh Exp $ */
/*
 * Mesa 3-D graphics library
 * Version:  3.3
 *
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Original Mesa / 3Dfx device driver (C) 1999 David Bucciarelli, by the
 * terms stated above.
 *
 * Thank you for your contribution, David!
 *
 * Please make note of the above copyright/license statement.  If you
 * contributed code or bug fixes to this code under the previous (GNU
 * Library) license and object to the new license, your code will be
 * removed at your request.  Please see the Mesa docs/COPYRIGHT file
 * for more information.
 *
 * Additional Mesa/3Dfx driver developers:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *   Keith Whitwell <keith@precisioninsight.com>
 *
 * See fxapi.h for more revision/author details.
 */


/* fxtexman.c - 3Dfx VooDoo texture memory functions */


#include "fxdrv.h"
#include "fxtexman.h"
#include "fxddtex.h"


#define BAD_ADDRESS ((FxU32) -1)


#ifdef TEXSANITY
static void
fubar(void)
{
}

/*
 * Sanity Check
 */
static void
sanity(fxMesaContext fxMesa)
{
    MemRange *tmp, *prev, *pos;

    prev = 0;
    tmp = fxMesa->tmFree[0];
    while (tmp) {
        if (!tmp->startAddr && !tmp->endAddr) {
            fprintf(stderr, "Textures fubar\n");
            fubar();
        }
        if (tmp->startAddr >= tmp->endAddr) {
            fprintf(stderr, "Node fubar\n");
            fubar();
        }
        if (prev && (prev->startAddr >= tmp->startAddr ||
                     prev->endAddr > tmp->startAddr)) {
            fprintf(stderr, "Sorting fubar\n");
            fubar();
        }
        prev = tmp;
        tmp = tmp->next;
    }
    prev = 0;
    tmp = fxMesa->tmFree[1];
    while (tmp) {
        if (!tmp->startAddr && !tmp->endAddr) {
            fprintf(stderr, "Textures fubar\n");
            fubar();
        }
        if (tmp->startAddr >= tmp->endAddr) {
            fprintf(stderr, "Node fubar\n");
            fubar();
        }
        if (prev && (prev->startAddr >= tmp->startAddr ||
                     prev->endAddr > tmp->startAddr)) {
            fprintf(stderr, "Sorting fubar\n");
            fubar();
        }
        prev = tmp;
        tmp = tmp->next;
    }
}
#endif


/*
 * Allocate and initialize a new MemRange struct.
 * Try to allocate it from the pool of free MemRange nodes rather than malloc.
 */
static MemRange *
NewRangeNode(fxMesaContext fxMesa, FxU32 start, FxU32 end)
{
    struct gl_shared_state *mesaShared = fxMesa->glCtx->Shared;
    struct TdfxSharedState *shared = (struct TdfxSharedState *) mesaShared->DriverData;
    MemRange *result;

    _glthread_LOCK_MUTEX(mesaShared->Mutex);
    if (shared && shared->tmPool) {
        result = shared->tmPool;
        shared->tmPool = shared->tmPool->next;
    }
    else {
        result = MALLOC(sizeof(MemRange));

    }
    _glthread_UNLOCK_MUTEX(mesaShared->Mutex);

    if (!result) {
        /*fprintf(stderr, "fxDriver: out of memory!\n");*/
        return NULL;
    }

    result->startAddr = start;
    result->endAddr = end;
    result->next = NULL;

    return result;
}


/*
 * Delete a MemRange struct.
 * We keep a linked list of free/available MemRange structs to
 * avoid extra malloc/free calls.
 */
#if 0
static void
DeleteRangeNode_NoLock(struct TdfxSharedState *shared, MemRange *range)
{
    /* insert at head of list */
    range->next = shared->tmPool;
    shared->tmPool = range;
}
#endif

#define DELETE_RANGE_NODE(shared, range) \
    (range)->next = (shared)->tmPool;    \
    (shared)->tmPool = (range)


/*
 * When we've run out of texture memory we have to throw out an
 * existing texture to make room for the new one.  This function
 * determins the texture to throw out.
 */
static struct gl_texture_object *
FindOldestObject(fxMesaContext fxMesa, FxU32 tmu)
{
    const GLuint bindnumber = fxMesa->texBindNumber;
    struct gl_texture_object *oldestObj, *obj, *lowestPriorityObj;
    GLfloat lowestPriority;
    GLuint oldestAge;

    oldestObj = NULL;
    oldestAge = 0;

    lowestPriority = 1.0F;
    lowestPriorityObj = NULL;

    for (obj = fxMesa->glCtx->Shared->TexObjectList; obj; obj = obj->Next) {
        tfxTexInfo *info = fxTMGetTexInfo(obj);

        if (info && info->isInTM &&
            ((info->whichTMU == tmu) || (info->whichTMU == FX_TMU_BOTH) ||
             (info->whichTMU == FX_TMU_SPLIT))) {
            GLuint age, lasttime;

            lasttime = info->lastTimeUsed;

            if (lasttime > bindnumber)
                age = bindnumber + (UINT_MAX - lasttime + 1); /* TO DO: check wrap around */
            else
                age = bindnumber - lasttime;

            if (age >= oldestAge) {
                oldestAge = age;
                oldestObj = obj;
            }

            /* examine priority */
            if (obj->Priority < lowestPriority) {
                lowestPriority = obj->Priority;
                lowestPriorityObj = obj;
            }
        }
    }

    if (lowestPriority < 1.0) {
        ASSERT(lowestPriorityObj);
        /*
        printf("discard %d pri=%f\n", lowestPriorityObj->Name, lowestPriority);
        */
        return lowestPriorityObj;
    }
    else {
        /*
        printf("discard %d age=%d\n", oldestObj->Name, oldestAge);
        */
        return oldestObj;
    }
}


/*
 * Find the address (offset?) at which we can store a new texture.
 * <tmu> is the texture unit.
 * <size> is the texture size in bytes.
 */
static FxU32
FindStartAddr(fxMesaContext fxMesa, FxU32 tmu, FxU32 size)
{
    struct gl_shared_state *mesaShared = fxMesa->glCtx->Shared;
    struct TdfxSharedState *shared = (struct TdfxSharedState *) mesaShared->DriverData;
    MemRange *prev, *block;
    FxU32 result;
    struct gl_texture_object *obj;

    if (shared->umaTexMemory) {
        assert(tmu == FX_TMU0);
    }

    _glthread_LOCK_MUTEX(mesaShared->Mutex);
    while (1) {
        prev = NULL;
        block = shared->tmFree[tmu];
        while (block) {
            if (block->endAddr - block->startAddr >= size) {
                /* The texture will fit here */
                result = block->startAddr;
                block->startAddr += size;
                if (block->startAddr == block->endAddr) {
                    /* Remove this node since it's empty */
                    if (prev) {
                        prev->next = block->next;
                    }
                    else {
                        shared->tmFree[tmu] = block->next;
                    }
                    DELETE_RANGE_NODE(shared, block);
                }
                shared->freeTexMem[tmu] -= size;
                _glthread_UNLOCK_MUTEX(mesaShared->Mutex);
                return result;
            }
            prev = block;
            block = block->next;
        }
        /* No free space. Discard oldest */
        obj = FindOldestObject(fxMesa, tmu);
        if (!obj) {
            /*gl_problem(NULL, "fx Driver: No space for texture\n");*/
            _glthread_UNLOCK_MUTEX(mesaShared->Mutex);
            return BAD_ADDRESS;
        }
        fxTMMoveOutTM_NoLock(fxMesa, obj);
        fxMesa->stats.texSwaps++;
    }

    /* never get here, but play it safe */
    _glthread_UNLOCK_MUTEX(mesaShared->Mutex);
    return BAD_ADDRESS;
}


/*
 * Remove the given MemRange node from hardware texture memory.
 */
static void
RemoveRange_NoLock(fxMesaContext fxMesa, FxU32 tmu, MemRange *range)
{
    struct gl_shared_state *mesaShared = fxMesa->glCtx->Shared;
    struct TdfxSharedState *shared = (struct TdfxSharedState *) mesaShared->DriverData;
    MemRange *block, *prev;

    if (shared->umaTexMemory) {
       assert(tmu == FX_TMU0);
    }

    if (!range)
        return;

    if (range->startAddr == range->endAddr) {
        DELETE_RANGE_NODE(shared, range);
        return;
    }
    shared->freeTexMem[tmu] += range->endAddr - range->startAddr;

    /* find position in linked list to insert this MemRange node */
    prev = NULL;
    block = shared->tmFree[tmu];
    while (block) {
        if (range->startAddr > block->startAddr) {
            prev = block;
            block = block->next;
        }
        else {
            break;
        }
    }

    /* Insert the free block, combine with adjacent blocks when possible */
    range->next = block;
    if (block) {
        if (range->endAddr == block->startAddr) {
            /* Combine */
            block->startAddr = range->startAddr;
            DELETE_RANGE_NODE(shared, range);
            range = block;
        }
    }
    if (prev) {
        if (prev->endAddr == range->startAddr) {
            /* Combine */
            prev->endAddr = range->endAddr;
            prev->next = range->next;
            DELETE_RANGE_NODE(shared, range);
        }
        else {
            prev->next = range;
        }
    }
    else {
        shared->tmFree[tmu] = range;
    }
}


static void
RemoveRange(fxMesaContext fxMesa, FxU32 tmu, MemRange *range)
{
    struct gl_shared_state *mesaShared = fxMesa->glCtx->Shared;
    _glthread_LOCK_MUTEX(mesaShared->Mutex);
    RemoveRange_NoLock(fxMesa, tmu, range);
    _glthread_UNLOCK_MUTEX(mesaShared->Mutex);
}


/*
 * Allocate space for a texture image.
 * <tmu> is the texture unit
 * <texmemsize> is the number of bytes to allocate
 */
static MemRange *
AllocTexMem(fxMesaContext fxMesa, FxU32 tmu, FxU32 texmemsize)
{
    FxU32 startAddr = FindStartAddr(fxMesa, tmu, texmemsize);
    if (startAddr == BAD_ADDRESS) {
        return NULL;
    }
    else {
        MemRange *range;
        range = NewRangeNode(fxMesa, startAddr, startAddr + texmemsize);
        return range;
    }
}


/*
 * Download (copy) the given texture data into the Voodoo's texture memory.
 * The texture memory must have already been allocated.
 * Called by fxTMMoveInTM_NoLock() and fxTMRestoreTextures().
 */
static void
DownloadTexture(fxMesaContext fxMesa, struct gl_texture_object *tObj)
{
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);
    int i, l;
    FxU32 targetTMU;

    assert(tObj);
    assert(ti);

    targetTMU = ti->whichTMU;

    switch (targetTMU) {
    case FX_TMU0:
    case FX_TMU1:
        if (ti->tm[targetTMU]) {
            for (i = FX_largeLodValue(ti->info), l = ti->minLevel;
                 i <= FX_smallLodValue(ti->info); i++, l++)
                FX_grTexDownloadMipMapLevel_NoLock(targetTMU,
                                                   ti->tm[targetTMU]->startAddr,
                                                   FX_valueToLod(i),
                                                   FX_largeLodLog2(ti->info),
                                                   FX_aspectRatioLog2(ti->info),
                                                   ti->info.format,
                                                   GR_MIPMAPLEVELMASK_BOTH,
                                                   ti->mipmapLevel[l].data);
        }
        break;
    case FX_TMU_SPLIT:
        if (ti->tm[FX_TMU0] && ti->tm[FX_TMU1]) {
            for (i = FX_largeLodValue(ti->info), l = ti->minLevel;
                 i <= FX_smallLodValue(ti->info); i++, l++) {
                FX_grTexDownloadMipMapLevel_NoLock(GR_TMU0,
                                                  ti->tm[FX_TMU0]->startAddr,
                                                  FX_valueToLod(i),
                                                  FX_largeLodLog2(ti->info),
                                                  FX_aspectRatioLog2(ti->info),
                                                  ti->info.format,
                                                  GR_MIPMAPLEVELMASK_ODD,
                                                  ti->mipmapLevel[l].data);

                FX_grTexDownloadMipMapLevel_NoLock(GR_TMU1,
                                                  ti->tm[FX_TMU1]->startAddr,
                                                  FX_valueToLod(i),
                                                  FX_largeLodLog2(ti->info),
                                                  FX_aspectRatioLog2(ti->info),
                                                  ti->info.format,
                                                  GR_MIPMAPLEVELMASK_EVEN,
                                                  ti->mipmapLevel[l].data);
            }
        }
        break;
    case FX_TMU_BOTH:
        if (ti->tm[FX_TMU0] && ti->tm[FX_TMU1]) {
            for (i = FX_largeLodValue(ti->info), l = ti->minLevel;
                 i <= FX_smallLodValue(ti->info); i++, l++) {
                FX_grTexDownloadMipMapLevel_NoLock(GR_TMU0,
                                                  ti->tm[FX_TMU0]->startAddr,
                                                  FX_valueToLod(i),
                                                  FX_largeLodLog2(ti->info),
                                                  FX_aspectRatioLog2(ti->info),
                                                  ti->info.format,
                                                  GR_MIPMAPLEVELMASK_BOTH,
                                                  ti->mipmapLevel[l].data);

                FX_grTexDownloadMipMapLevel_NoLock(GR_TMU1,
                                                  ti->tm[FX_TMU1]->startAddr,
                                                  FX_valueToLod(i),
                                                  FX_largeLodLog2(ti->info),
                                                  FX_aspectRatioLog2(ti->info),
                                                  ti->info.format,
                                                  GR_MIPMAPLEVELMASK_BOTH,
                                                  ti->mipmapLevel[l].data);
            }
        }
        break;
    default:
        fprintf(stderr,
            "fx Driver: internal error in DownloadTexture -> bad tmu (%d)\n",
            (int) targetTMU);
        return;  /* used to abort here */
    }
}



/*
 * Allocate space for the given texture in texture memory then
 * download (copy) it into that space.
 */
void
fxTMMoveInTM_NoLock(fxMesaContext fxMesa, struct gl_texture_object *tObj,
                    FxU32 targetTMU)
{
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);
    FxU32 texmemsize;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxTMMoveInTM(%d)\n", tObj->Name);
    }

    fxMesa->stats.reqTexUpload++;

    if (!ti->validated) {
        gl_problem(NULL,
            "fx Driver: internal error in fxTMMoveInTM() -> not validated\n");
        return;  /* used to abort here */
    }

    if (ti->isInTM) {
        if (ti->whichTMU == targetTMU)
            return;
        if (targetTMU == FX_TMU_SPLIT || ti->whichTMU == FX_TMU_SPLIT) {
            fxTMMoveOutTM_NoLock(fxMesa, tObj);
        }
        else {
            if (ti->whichTMU == FX_TMU_BOTH)
                return;
            targetTMU = FX_TMU_BOTH;
        }
    }

    if (MESA_VERBOSE & (VERBOSE_DRIVER | VERBOSE_TEXTURE)) {
        fprintf(stderr,
                "fxmesa: downloading %x (%d) in texture memory in %d\n",
                 tObj, tObj->Name, (int) targetTMU);
    }

    ti->whichTMU = targetTMU;

    switch (targetTMU) {
    case FX_TMU0:
    case FX_TMU1:
        texmemsize = FX_grTexTextureMemRequired_NoLock(GR_MIPMAPLEVELMASK_BOTH,
                                                       &(ti->info));
        ti->tm[targetTMU] = AllocTexMem(fxMesa, targetTMU, texmemsize);
        DownloadTexture(fxMesa, tObj);
        break;
    case FX_TMU_SPLIT:
        texmemsize = FX_grTexTextureMemRequired_NoLock(GR_MIPMAPLEVELMASK_ODD,
                                                       &(ti->info));
        ti->tm[FX_TMU0] = AllocTexMem(fxMesa, FX_TMU0, texmemsize);
        if (ti->tm[FX_TMU0])
           fxMesa->stats.memTexUpload += texmemsize;

        texmemsize = FX_grTexTextureMemRequired_NoLock(GR_MIPMAPLEVELMASK_EVEN,
                                                       &(ti->info));
        ti->tm[FX_TMU1] = AllocTexMem(fxMesa, FX_TMU1, texmemsize);
        DownloadTexture(fxMesa, tObj);
        break;
    case FX_TMU_BOTH:
        texmemsize = FX_grTexTextureMemRequired_NoLock(GR_MIPMAPLEVELMASK_BOTH,
                                                       &(ti->info));
        ti->tm[FX_TMU0] = AllocTexMem(fxMesa, FX_TMU0, texmemsize);
        if (ti->tm[FX_TMU0])
           fxMesa->stats.memTexUpload += texmemsize;

        texmemsize = FX_grTexTextureMemRequired_NoLock(GR_MIPMAPLEVELMASK_BOTH,
                                                       &(ti->info));
        ti->tm[FX_TMU1] = AllocTexMem(fxMesa, FX_TMU1, texmemsize);
        DownloadTexture(fxMesa, tObj);
        break;
    default:
        fprintf(stderr,
            "fx Driver: internal error in fxTMMoveInTM() -> bad tmu (%d)\n",
            (int) targetTMU);
        return;  /* used to abort here */
    }

    fxMesa->stats.texUpload++;

    ti->isInTM = GL_TRUE;
}


void
fxTMMoveInTM(fxMesaContext fxMesa, struct gl_texture_object *tObj,
             FxU32 targetTMU)
{
    BEGIN_BOARD_LOCK(fxMesa);
    fxTMMoveInTM_NoLock(fxMesa, tObj, targetTMU);
    END_BOARD_LOCK(fxMesa);
}


void
fxTMReloadMipMapLevel(GLcontext *ctx, struct gl_texture_object *tObj,
                      GLint level)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);
    GrLOD_t lodlevel;
    FxU32 tmu;

    if (!ti->validated) {
        gl_problem(ctx, "internal error in fxTMReloadMipMapLevel() -> not validated\n");
        return;
    }

    tmu = ti->whichTMU;
    fxTMMoveInTM(fxMesa, tObj, tmu);

    fxTexGetInfo(ctx, ti->mipmapLevel[0].width, ti->mipmapLevel[0].height,
                 &lodlevel, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

#ifdef FX_GLIDE3
    lodlevel -= level;
#else
    lodlevel += level;
#endif
    switch (tmu) {
    case FX_TMU0:
    case FX_TMU1:
        FX_grTexDownloadMipMapLevel(fxMesa, tmu,
                                    ti->tm[tmu]->startAddr,
                                    FX_valueToLod(FX_lodToValue(lodlevel)),
                                    FX_largeLodLog2(ti->info),
                                    FX_aspectRatioLog2(ti->info),
                                    ti->info.format,
                                    GR_MIPMAPLEVELMASK_BOTH,
                                    ti->mipmapLevel[level].data);
        break;
    case FX_TMU_SPLIT:
        FX_grTexDownloadMipMapLevel(fxMesa, GR_TMU0,
                                    ti->tm[GR_TMU0]->startAddr,
                                    FX_valueToLod(FX_lodToValue(lodlevel)),
                                    FX_largeLodLog2(ti->info),
                                    FX_aspectRatioLog2(ti->info),
                                    ti->info.format,
                                    GR_MIPMAPLEVELMASK_ODD,
                                    ti->mipmapLevel[level].data);

        FX_grTexDownloadMipMapLevel(fxMesa, GR_TMU1,
                                    ti->tm[GR_TMU1]->startAddr,
                                    FX_valueToLod(FX_lodToValue(lodlevel)),
                                    FX_largeLodLog2(ti->info),
                                    FX_aspectRatioLog2(ti->info),
                                    ti->info.format,
                                    GR_MIPMAPLEVELMASK_EVEN,
                                    ti->mipmapLevel[level].data);
        break;
    case FX_TMU_BOTH:
        FX_grTexDownloadMipMapLevel(fxMesa, GR_TMU0,
                                    ti->tm[GR_TMU0]->startAddr,
                                    FX_valueToLod(FX_lodToValue(lodlevel)),
                                    FX_largeLodLog2(ti->info),
                                    FX_aspectRatioLog2(ti->info),
                                    ti->info.format,
                                    GR_MIPMAPLEVELMASK_BOTH,
                                    ti->mipmapLevel[level].data);

        FX_grTexDownloadMipMapLevel(fxMesa, GR_TMU1,
                                    ti->tm[GR_TMU1]->startAddr,
                                    FX_valueToLod(FX_lodToValue(lodlevel)),
                                    FX_largeLodLog2(ti->info),
                                    FX_aspectRatioLog2(ti->info),
                                    ti->info.format,
                                    GR_MIPMAPLEVELMASK_BOTH,
                                    ti->mipmapLevel[level].data);
        break;

    default:
        fprintf(stderr,
                "fx Driver: internal error in fxTMReloadMipMapLevel() -> wrong tmu (%d)\n",
                (int) tmu);
        break;
    }
}

#if	0
/*
 * This doesn't work.  It can't work for compressed textures.
 */
void
fxTMReloadSubMipMapLevel(GLcontext *ctx,
                         struct gl_texture_object *tObj,
                         GLint level, GLint yoffset, GLint height)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);
    GrLOD_t lodlevel;
    unsigned short *data;
    GLint tmu;

    if (!ti->validated) {
       gl_problem(ctx, "fx Driver: internal error in fxTMReloadSubMipMapLevel() -> not validated\n");
       return;
    }

    tmu = (int) ti->whichTMU;
    fxTMMoveInTM(fxMesa, tObj, tmu);

    fxTexGetInfo(ctx, ti->mipmapLevel[0].width, ti->mipmapLevel[0].height,
                 &lodlevel, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    data = ti->mipmapLevel[level].data +
        yoffset * ti->mipmapLevel[level].width *
        ti->mipmapLevel[level].texelSize;

    switch (tmu) {
    case FX_TMU0:
    case FX_TMU1:
        FX_grTexDownloadMipMapLevelPartial(fxMesa, tmu,
                                           ti->tm[tmu]->startAddr,
                                           FX_valueToLod(FX_lodToValue
                                                         (lodlevel) + level),
                                           FX_largeLodLog2(ti->info),
                                           FX_aspectRatioLog2(ti->info),
                                           ti->info.format,
                                           GR_MIPMAPLEVELMASK_BOTH, data,
                                           yoffset, yoffset + height - 1);
        break;
    case FX_TMU_SPLIT:
        FX_grTexDownloadMipMapLevelPartial(fxMesa, GR_TMU0,
                                           ti->tm[FX_TMU0]->startAddr,
                                           FX_valueToLod(FX_lodToValue
                                                         (lodlevel) + level),
                                           FX_largeLodLog2(ti->info),
                                           FX_aspectRatioLog2(ti->info),
                                           ti->info.format,
                                           GR_MIPMAPLEVELMASK_ODD, data,
                                           yoffset, yoffset + height - 1);

        FX_grTexDownloadMipMapLevelPartial(fxMesa, GR_TMU1,
                                           ti->tm[FX_TMU1]->startAddr,
                                           FX_valueToLod(FX_lodToValue
                                                         (lodlevel) + level),
                                           FX_largeLodLog2(ti->info),
                                           FX_aspectRatioLog2(ti->info),
                                           ti->info.format,
                                           GR_MIPMAPLEVELMASK_EVEN, data,
                                           yoffset, yoffset + height - 1);
        break;
    case FX_TMU_BOTH:
        FX_grTexDownloadMipMapLevelPartial(fxMesa, GR_TMU0,
                                           ti->tm[FX_TMU0]->startAddr,
                                           FX_valueToLod(FX_lodToValue
                                                         (lodlevel) + level),
                                           FX_largeLodLog2(ti->info),
                                           FX_aspectRatioLog2(ti->info),
                                           ti->info.format,
                                           GR_MIPMAPLEVELMASK_BOTH, data,
                                           yoffset, yoffset + height - 1);

        FX_grTexDownloadMipMapLevelPartial(fxMesa, GR_TMU1,
                                           ti->tm[FX_TMU1]->startAddr,
                                           FX_valueToLod(FX_lodToValue
                                                         (lodlevel) + level),
                                           FX_largeLodLog2(ti->info),
                                           FX_aspectRatioLog2(ti->info),
                                           ti->info.format,
                                           GR_MIPMAPLEVELMASK_BOTH, data,
                                           yoffset, yoffset + height - 1);
        break;
    default:
        fprintf(stderr,
                "fx Driver: internal error in fxTMReloadSubMipMapLevel() -> wrong tmu (%d)\n",
                tmu);
        return;
    }
}
#endif


/*
 * Move the given texture out of hardware texture memory.
 * This deallocates the texture's memory space.
 */
void
fxTMMoveOutTM_NoLock(fxMesaContext fxMesa, struct gl_texture_object *tObj)
{
    struct gl_shared_state *mesaShared = fxMesa->glCtx->Shared;
    struct TdfxSharedState *shared = (struct TdfxSharedState *) mesaShared->DriverData;
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxTMMoveOutTM(%x (%d))\n", 
		tObj, tObj->Name);
    }

    if (!ti->isInTM)
        return;

    switch (ti->whichTMU) {
    case FX_TMU0:
    case FX_TMU1:
        RemoveRange_NoLock(fxMesa, ti->whichTMU, ti->tm[ti->whichTMU]);
        break;
    case FX_TMU_SPLIT:
    case FX_TMU_BOTH:
        assert(!shared->umaTexMemory);
        RemoveRange_NoLock(fxMesa, FX_TMU0, ti->tm[FX_TMU0]);
        RemoveRange_NoLock(fxMesa, FX_TMU1, ti->tm[FX_TMU1]);
        break;
    default:
        fprintf(stderr, "fx Driver: internal error in fxTMMoveOutTM()\n");
        return;
    }

    ti->isInTM = GL_FALSE;
    ti->whichTMU = FX_TMU_NONE;
}

void
fxTMMoveOutTM(fxMesaContext fxMesa, struct gl_texture_object *tObj)
{
    struct gl_shared_state *mesaShared = fxMesa->glCtx->Shared;
    struct TdfxSharedState *shared = (struct TdfxSharedState *) mesaShared->DriverData;
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxTMMoveOutTM(%x (%d))\n", 
		tObj, tObj->Name);
    }

    if (!ti->isInTM)
        return;

    switch (ti->whichTMU) {
    case FX_TMU0:
    case FX_TMU1:
        RemoveRange(fxMesa, ti->whichTMU, ti->tm[ti->whichTMU]);
        break;
    case FX_TMU_SPLIT:
    case FX_TMU_BOTH:
        assert(!shared->umaTexMemory);
        RemoveRange(fxMesa, FX_TMU0, ti->tm[FX_TMU0]);
        RemoveRange(fxMesa, FX_TMU1, ti->tm[FX_TMU1]);
        break;
    default:
        fprintf(stderr, "fx Driver: internal error in fxTMMoveOutTM()\n");
        return;
    }

    ti->isInTM = GL_FALSE;
    ti->whichTMU = FX_TMU_NONE;
}


/*
 * Called via glDeleteTexture to delete a texture object.
 */
void
fxTMFreeTexture(fxMesaContext fxMesa, struct gl_texture_object *tObj)
{
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);
    if (ti) {
        int i;
        fxTMMoveOutTM(fxMesa, tObj);
        for (i = 0; i < MAX_TEXTURE_LEVELS; i++) {
            if (ti->mipmapLevel[i].data) {
                FREE(ti->mipmapLevel[i].data);
                ti->mipmapLevel[i].data = NULL;
            }
        }
        FREE(ti);
        tObj->DriverData = NULL;
    }
}


/*
 * Initialize texture memory.
 * We take care of one or both TMU's here.
 */
void
fxTMInit(fxMesaContext fxMesa)
{
    if (!fxMesa->glCtx->Shared->DriverData) {
        const char *extensions;
        struct TdfxSharedState *shared = CALLOC_STRUCT(TdfxSharedState);
        if (!shared)
           return;

        extensions = FX_grGetString(fxMesa, GR_EXTENSION);
        if (strstr(extensions, "TEXUMA")) {
            FxU32 start, end;
            shared->umaTexMemory = GL_TRUE;
            FX_grEnable(fxMesa, GR_TEXTURE_UMA_EXT);
            start = FX_grTexMinAddress(fxMesa, 0);
            end = FX_grTexMaxAddress(fxMesa, 0);
            shared->totalTexMem[0] = end - start;
            shared->totalTexMem[1] = 0;
            shared->freeTexMem[0] = end - start;
            shared->tmFree[0] = NewRangeNode(fxMesa, start, end);
            /*printf("UMA tex memory: %d\n", (int) (end - start));*/
        }
        else {
            const int numTMUs = fxMesa->haveTwoTMUs ? 2 : 1;
            int tmu;
            shared->umaTexMemory = GL_FALSE;
            for (tmu = 0; tmu < numTMUs; tmu++) {
                FxU32 start = FX_grTexMinAddress(fxMesa, tmu);
                FxU32 end = FX_grTexMaxAddress(fxMesa, tmu);
                shared->totalTexMem[tmu] = end - start;
                shared->freeTexMem[tmu] = end - start;
                shared->tmFree[tmu] = NewRangeNode(fxMesa, start, end);
                /*printf("Split tex memory: %d\n", (int) (end - start));*/
            }
        }

        shared->tmPool = NULL;
        fxMesa->glCtx->Shared->DriverData = shared;
        /*printf("Texture memory init UMA: %d\n", shared->umaTexMemory);*/
    }        
}


/*
 * Clean-up texture memory before destroying context.
 */
void
fxTMClose(fxMesaContext fxMesa)
{
    if (fxMesa->glCtx->Shared->RefCount == 1) {
        /* refcount will soon go to zero, free our 3dfx stuff */
        struct TdfxSharedState *shared = (struct TdfxSharedState *) fxMesa->glCtx->Shared->DriverData;

        const int numTMUs = fxMesa->haveTwoTMUs ? 2 : 1;
        int tmu;
        MemRange *tmp, *next;

        /* Deallocate the pool of free MemRange nodes */
        tmp = shared->tmPool;
        while (tmp) {
            next = tmp->next;
            FREE(tmp);
            tmp = next;
        }

        /* Delete the texture memory block MemRange nodes */
        for (tmu = 0; tmu < numTMUs; tmu++) {
            tmp = shared->tmFree[tmu];
            while (tmp) {
                next = tmp->next;
                FREE(tmp);
                tmp = next;
            }
        }

        FREE(shared);
        fxMesa->glCtx->Shared->DriverData = NULL;
    }
}


/*
 * After a context switch this function will be called to restore
 * texture memory for the new context.
 */
void
fxTMRestoreTextures_NoLock(fxMesaContext ctx)
{
    struct gl_texture_object *tObj;

    for (tObj = ctx->glCtx->Shared->TexObjectList; tObj; tObj = tObj->Next) {
        tfxTexInfo *ti = fxTMGetTexInfo(tObj);
        if (ti && ti->isInTM) {
            int i;
            for (i = 0; i < MAX_TEXTURE_UNITS; i++) {
                if (ctx->glCtx->Texture.Unit[i].Current == tObj) {
                    DownloadTexture(ctx, tObj);
                    break;
                }
            }
            if (i == MAX_TEXTURE_UNITS) {
                fxTMMoveOutTM_NoLock(ctx, tObj);
            }
        }
    }
}
