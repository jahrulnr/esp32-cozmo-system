/*
 * Copyright (C) 2008-2009 SVOX AG, Baslerstr. 30, 8048 Zuerich, Switzerland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @file picorsrc.c
 *
 * Copyright (C) 2008-2009 SVOX AG, Baslerstr. 30, 8048 Zuerich, Switzerland
 * All rights reserved.
 *
 * History:
 * - 2009-04-20 -- initial version
 *
 */

#ifndef PICORSRC_H_
#define PICORSRC_H_

#include "picodefs.h"
#include "picoos.h"
#include "picodbg.h"

/* knowledge layer */
#include "picoknow.h"

#include "picokdt.h"
#include "picoklex.h"
#include "picokfst.h"
#include "picokpdf.h"
#include "picoktab.h"
#include "picokpr.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif


#define PICORSRC_MAX_RSRC_NAME_SIZ PICO_MAX_RESOURCE_NAME_SIZE /* including terminating NULLC */

#define PICORSRC_MAX_NUM_VOICES 64

/* size of kb array of a voice */
#define PICORSRC_KB_ARRAY_SIZE 64

typedef picoos_char picorsrc_resource_name_t[PICORSRC_MAX_RSRC_NAME_SIZ];

typedef enum picorsrc_resource_type {
    PICORSRC_TYPE_NULL,
    PICORSRC_TYPE_TEXTANA,
    PICORSRC_TYPE_SIGGEN,
    PICORSRC_TYPE_USER_LEX,
    PICORSRC_TYPE_USER_PREPROC,
    PICORSRC_TYPE_OTHER
} picorsrc_resource_type_t;


#define PICORSRC_FIELD_VALUE_TEXTANA (picoos_char *) "TEXTANA"
#define PICORSRC_FIELD_VALUE_SIGGEN (picoos_char *) "SIGGEN"
#define PICORSRC_FIELD_VALUE_USERLEX (picoos_char *) "USERLEX"
#define PICORSRC_FIELD_VALUE_USERTPP (picoos_char *) "USERTPP"



typedef struct picorsrc_resource_manager * picorsrc_ResourceManager;
typedef struct picorsrc_voice            * picorsrc_Voice;
typedef struct picorsrc_resource         * picorsrc_Resource;


/* **************************************************************************
 *
 *          file name extensions
 *
 ****************************************************************************/

#define PICO_BIN_EXTENSION      ".bin"
#define PICO_INPLACE_EXTENSION  ".inp"



/* **************************************************************************
 *
 *          construct/destruct resource manager
 *
 ****************************************************************************/

/* create resource manager, given a config file name (or default name, if empty) */

picorsrc_ResourceManager picorsrc_newResourceManager(picoos_MemoryManager mm, picoos_Common common /* , picoos_char * configFile */);

void picorsrc_disposeResourceManager(picoos_MemoryManager mm, picorsrc_ResourceManager * this);


/* **************************************************************************
 *
 *          resources
 *
 ****************************************************************************/

/**
 * Returns non-zero if 'resource' is a valid resource handle, zero otherwise.
 */
picoos_int16 picoctrl_isValidResourceHandle(picorsrc_Resource resource);

/* load resource file. the type of resource file, magic numbers, checksum etc. are in the header, then follows the directory
 * (with fixed structure per resource type), then the knowledge bases themselves (as byte streams) */
pico_status_t picorsrc_loadResource(picorsrc_ResourceManager this,
        picoos_char * fileName, picorsrc_Resource * resource);

/* unload resource file. (warn if resource file is busy) */
pico_status_t picorsrc_unloadResource(picorsrc_ResourceManager this, picorsrc_Resource * rsrc);


pico_status_t picorsrc_createDefaultResource(picorsrc_ResourceManager this /*,
        picorsrc_Resource * resource */);


pico_status_t picorsrc_rsrcGetName(picorsrc_Resource resource,
        picoos_char * name, picoos_uint32 maxlen);

/* **************************************************************************
 *
 *          voice definitions
 *
 ****************************************************************************/


pico_status_t picorsrc_createVoiceDefinition(picorsrc_ResourceManager this,
        picoos_char * voiceName);


pico_status_t picorsrc_releaseVoiceDefinition(picorsrc_ResourceManager this,
        picoos_char * voiceName);

pico_status_t picorsrc_addResourceToVoiceDefinition(picorsrc_ResourceManager this,
        picoos_char * voiceName, picoos_char * resourceName);

/* **************************************************************************
 *
 *          voices
 *
 ****************************************************************************/

/**  object   : Voice
 *   shortcut : voice
 *
 */

typedef struct picorsrc_voice {

    picorsrc_Voice next;

    picoknow_KnowledgeBase kbArray[PICORSRC_KB_ARRAY_SIZE];

    picoos_uint8 numResources;

    picorsrc_Resource resourceArray[PICO_MAX_NUM_RSRC_PER_VOICE];


} picorsrc_voice_t;



/* create voice, given a voice name. the corresponding lock counts are incremented */
pico_status_t picorsrc_createVoice(picorsrc_ResourceManager this, const picoos_char * voiceName, picorsrc_Voice * voice);

/* dispose voice. the corresponding lock counts are decremented. */
pico_status_t picorsrc_releaseVoice(picorsrc_ResourceManager this, picorsrc_Voice * voice);

// hhhhhhhhhhhhhhhhhh

#if 0
}
#endif


#if defined(PICO_DEBUG)
#include "picokdbg.h"
#endif


/**  object   : Resource
 *   shortcut : rsrc
 *
 */
typedef struct picorsrc_resource {
    picoos_uint32 magic;  /* magic number used to validate handles */
    /* next connects all active resources of a resource manager and the garbaged resources of the manager's free list */
    picorsrc_Resource next;
    picorsrc_resource_type_t type;
    picorsrc_resource_name_t name;
    picoos_int8 lockCount;  /* count of current subscribers of this resource */
    picoos_File file;
    picoos_uint8 * raw_mem; /* pointer to allocated memory. NULL if preallocated. */
    /* picoos_uint32 size; */
    picoos_uint8 * start; /* start of content (after header) */
    picoknow_KnowledgeBase kbList;
} picorsrc_resource_t;


#define MAGIC_MASK 0x7049634F  /* pIcO */

#define SET_MAGIC_NUMBER(res) \
    (res)->magic = ((picoos_uint32) (uintptr_t) (res)) ^ MAGIC_MASK

#define CHECK_MAGIC_NUMBER(res) \
    ((res)->magic == (((picoos_uint32) (uintptr_t) (res)) ^ MAGIC_MASK))


static picorsrc_Resource picorsrc_newResource(picoos_MemoryManager mm)
{
    picorsrc_Resource this = picoos_allocate(mm, sizeof(*this));
    if (NULL != this) {
        SET_MAGIC_NUMBER(this);
        /* initialize */
        this->name[0] = NULLC;
        /* picoos_strlcpy(this->name, name,PICORSRC_MAX_RSRC_NAME_SIZ); */
        this->next = NULL;
        this->type = PICORSRC_TYPE_NULL;
        this->lockCount = 0;
        this->file = NULL;
        this->raw_mem = NULL;
        this->start = NULL;
        this->kbList = NULL;
        /* this->size=0; */
    }
    return this;
}

static void picorsrc_disposeResource(picoos_MemoryManager mm, picorsrc_Resource * this)
{
    if (NULL != (*this)) {
        (*this)->magic ^= 0xFFFEFDFC;
        /* we have to explicitly free 'raw_mem' here because in testing
           scenarios (where memory protection functionality is enabled)
           it might be allocated aside from normal memory */
        if ((*this)->raw_mem != NULL) {
            picoos_deallocProtMem(mm, (void *) &(*this)->raw_mem);
        }
        picoos_deallocate(mm,(void * *)this);
    }
}




static void picorsrc_initializeVoice(picorsrc_Voice this)
{
    picoos_uint16 i;
    if (NULL != this) {
        /* initialize */
        for (i=0; i<PICORSRC_KB_ARRAY_SIZE; i++) {
          this->kbArray[i] = NULL;
        }
        this->numResources = 0;
        this->next = NULL;
    }
}

static picorsrc_Voice picorsrc_newVoice(picoos_MemoryManager mm)
{
    picorsrc_Voice this = (picorsrc_Voice) picoos_allocate(mm,sizeof(*this));
    picorsrc_initializeVoice(this);
    return this;
}

/*
static void picorsrc_disposeVoice(picoos_MemoryManager mm, picorsrc_Voice * this)
{
    if (NULL != (*this)) {

        picoos_deallocate(mm,(void *)this);
    }
}
*/


/**  object   : VoiceDefinition
 *   shortcut : vdef
 *
 */

typedef struct picorsrc_voice_definition * picorsrc_VoiceDefinition;

typedef struct picorsrc_voice_definition {
    picoos_char voiceName[PICO_MAX_VOICE_NAME_SIZE];
    picoos_uint8 numResources;
    picorsrc_resource_name_t resourceName[PICO_MAX_NUM_RSRC_PER_VOICE];
    picorsrc_VoiceDefinition next;
} picorsrc_voice_definition_t;


static picorsrc_VoiceDefinition picorsrc_newVoiceDefinition(picoos_MemoryManager mm)
{
    /* picoos_uint8 i; */
    picorsrc_VoiceDefinition this = (picorsrc_VoiceDefinition) picoos_allocate(mm,sizeof(*this));
    if (NULL != this) {
        /* initialize */
        this->voiceName[0] = NULLC;
        this->numResources = 0;
        /*
        for (i=0; i < PICO_MAX_NUM_RSRC_PER_VOICE; i++) {
            this->resourceName[i][0] = NULLC;
        }
        */
        this->next = NULL;
    }
    return this;
}

/*
static void picorsrc_disposeVoiceDefinition(picoos_MemoryManager mm, picorsrc_VoiceDefinition * this)
{
    if (NULL != (*this)) {

        picoos_deallocate(mm,(void *)this);
    }
}
*/



/**  object   : ResourceManager
 *   shortcut : rm
 *
 */
typedef struct picorsrc_resource_manager {
    picoos_Common common;
    picoos_uint16 numResources;
    picorsrc_Resource resources, freeResources;
    picoos_uint16 numVoices;
    picorsrc_Voice voices, freeVoices;
    picoos_uint16 numVdefs;
    picorsrc_VoiceDefinition vdefs, freeVdefs;
    picoos_uint16 numKbs;
    picoknow_KnowledgeBase freeKbs;
    picoos_header_string_t tmpHeader;
} picorsrc_resource_manager_t;

pico_status_t picorsrc_createDefaultResource(picorsrc_ResourceManager this /*,
        picorsrc_Resource * resource */);


/* ******* accessing resources **************************************/


static pico_status_t findResource(picorsrc_ResourceManager this, picoos_char * resourceName, picorsrc_Resource * rsrc) {
    picorsrc_Resource r;
    if (NULL == this) {
        return PICO_ERR_NULLPTR_ACCESS;
    }
    r = this->resources;
    while (NULL != r && (0 != picoos_strcmp(r->name,resourceName))) {
        r = r->next;
    }
    *rsrc = r;
    return PICO_OK;
}

static picoos_uint8 isResourceLoaded(picorsrc_ResourceManager this, picoos_char * resourceName) {
    picorsrc_Resource res;

    if (PICO_OK == findResource(this, resourceName,&res)){
        return (NULL != res);
    } else {
        return FALSE;
    }
 }

static pico_status_t parse_resource_name(picoos_char * fileName)
{
    PICODBG_DEBUG(("analysing file name %s",fileName));
    if (picoos_has_extension(fileName,
            (picoos_char *)PICO_BIN_EXTENSION)) {
        return PICO_OK;
    } else {
        return PICO_EXC_UNEXPECTED_FILE_TYPE;
    }
}



static pico_status_t readHeader(picorsrc_ResourceManager this,
        picoos_FileHeader header, picoos_uint32 * headerlen, picoos_File file)
{

    picoos_uint16 hdrlen1;
    picoos_uint32 n;
    pico_status_t status;


    /* read PICO header */
    status = picoos_readPicoHeader(file, headerlen);
    if (PICO_OK == status) {

    } else {
        return picoos_emRaiseException(this->common->em,status,NULL,(picoos_char *)"problem reading file header");
    }
    /* read header length (excluding length itself) */
    status = picoos_read_pi_uint16(file,&hdrlen1);
    PICODBG_DEBUG(("got header size %d",hdrlen1));

    if (PICO_OK == status) {
        *headerlen += 2;
        status = (hdrlen1 <= PICOOS_MAX_HEADER_STRING_LEN-1) ? PICO_OK : PICO_ERR_OTHER;
        if (PICO_OK == status) {
            n = hdrlen1;
            if (picoos_ReadBytes(file, (picoos_uint8 *) this->tmpHeader, &n) && hdrlen1 == n) {
                this->tmpHeader[hdrlen1] = NULLC;
                *headerlen += hdrlen1;
                PICODBG_DEBUG(("got header <%s>",this->tmpHeader));

                status = PICO_OK;
            } else {
                status = PICO_ERR_OTHER;
            }
        }
        if (PICO_OK == status) {
            status = picoos_hdrParseHeader(header, this->tmpHeader);
        }
    }
    return status;
}

static pico_status_t picorsrc_createKnowledgeBase(
        picorsrc_ResourceManager this,
        picoos_uint8 * data,
        picoos_uint32 size,
        picoknow_kb_id_t kbid,
        picoknow_KnowledgeBase * kb)
{
    (*kb) = picoknow_newKnowledgeBase(this->common->mm);
    if (NULL == (*kb)) {
        return PICO_EXC_OUT_OF_MEM;
    }
    (*kb)->base = data;
    (*kb)->size = size;
    (*kb)->id = kbid;
    switch (kbid) {
        case PICOKNOW_KBID_TPP_MAIN:
        case PICOKNOW_KBID_TPP_USER_1:
        case PICOKNOW_KBID_TPP_USER_2:
            return picokpr_specializePreprocKnowledgeBase(*kb, this->common);
            break;
        case PICOKNOW_KBID_TAB_GRAPHS:
            return picoktab_specializeGraphsKnowledgeBase(*kb, this->common);
            break;
        case PICOKNOW_KBID_TAB_PHONES:
            return picoktab_specializePhonesKnowledgeBase(*kb, this->common);
            break;
        case PICOKNOW_KBID_TAB_POS:
            return picoktab_specializePosKnowledgeBase(*kb, this->common);
            break;
        case PICOKNOW_KBID_FIXED_IDS:
            return picoktab_specializeIdsKnowledgeBase(*kb, this->common);
            break;
        case PICOKNOW_KBID_LEX_MAIN:
        case PICOKNOW_KBID_LEX_USER_1:
        case PICOKNOW_KBID_LEX_USER_2:
            return picoklex_specializeLexKnowledgeBase(*kb, this->common);
            break;
        case PICOKNOW_KBID_DT_POSP:
            return picokdt_specializeDtKnowledgeBase(*kb, this->common,
                                                     PICOKDT_KDTTYPE_POSP);
            break;
        case PICOKNOW_KBID_DT_POSD:
            return picokdt_specializeDtKnowledgeBase(*kb, this->common,
                                                     PICOKDT_KDTTYPE_POSD);
            break;
        case PICOKNOW_KBID_DT_G2P:
            return picokdt_specializeDtKnowledgeBase(*kb, this->common,
                                                     PICOKDT_KDTTYPE_G2P);
            break;
        case PICOKNOW_KBID_DT_PHR:
            return picokdt_specializeDtKnowledgeBase(*kb, this->common,
                                                     PICOKDT_KDTTYPE_PHR);
            break;
        case PICOKNOW_KBID_DT_ACC:
             return picokdt_specializeDtKnowledgeBase(*kb, this->common,
                                                      PICOKDT_KDTTYPE_ACC);
             break;
        case PICOKNOW_KBID_FST_SPHO_1:
        case PICOKNOW_KBID_FST_SPHO_2:
        case PICOKNOW_KBID_FST_SPHO_3:
        case PICOKNOW_KBID_FST_SPHO_4:
        case PICOKNOW_KBID_FST_SPHO_5:
        case PICOKNOW_KBID_FST_SPHO_6:
        case PICOKNOW_KBID_FST_SPHO_7:
        case PICOKNOW_KBID_FST_SPHO_8:
        case PICOKNOW_KBID_FST_SPHO_9:
        case PICOKNOW_KBID_FST_SPHO_10:
        case PICOKNOW_KBID_FST_WPHO_1:
        case PICOKNOW_KBID_FST_WPHO_2:
        case PICOKNOW_KBID_FST_WPHO_3:
        case PICOKNOW_KBID_FST_WPHO_4:
        case PICOKNOW_KBID_FST_WPHO_5:
        case PICOKNOW_KBID_FST_SVOXPA_PARSE:
        case PICOKNOW_KBID_FST_XSAMPA_PARSE:
        case PICOKNOW_KBID_FST_XSAMPA2SVOXPA:

             return picokfst_specializeFSTKnowledgeBase(*kb, this->common);
             break;

        case PICOKNOW_KBID_DT_DUR:
        case PICOKNOW_KBID_DT_LFZ1:
        case PICOKNOW_KBID_DT_LFZ2:
        case PICOKNOW_KBID_DT_LFZ3:
        case PICOKNOW_KBID_DT_LFZ4:
        case PICOKNOW_KBID_DT_LFZ5:
        case PICOKNOW_KBID_DT_MGC1:
        case PICOKNOW_KBID_DT_MGC2:
        case PICOKNOW_KBID_DT_MGC3:
        case PICOKNOW_KBID_DT_MGC4:
        case PICOKNOW_KBID_DT_MGC5:
            return picokdt_specializeDtKnowledgeBase(*kb, this->common,
                                                     PICOKDT_KDTTYPE_PAM);
            break;
        case PICOKNOW_KBID_PDF_DUR:
            return picokpdf_specializePdfKnowledgeBase(*kb, this->common,
                                                       PICOKPDF_KPDFTYPE_DUR);

            break;
        case PICOKNOW_KBID_PDF_LFZ:
            return picokpdf_specializePdfKnowledgeBase(*kb, this->common,
                                                       PICOKPDF_KPDFTYPE_MUL);
            break;
        case PICOKNOW_KBID_PDF_MGC:
            return picokpdf_specializePdfKnowledgeBase(*kb, this->common,
                                                       PICOKPDF_KPDFTYPE_MUL);
            break;
        case PICOKNOW_KBID_PDF_PHS:
            return picokpdf_specializePdfKnowledgeBase(*kb, this->common,
                                                       PICOKPDF_KPDFTYPE_PHS);
            break;



#if defined(PICO_DEBUG)
        case PICOKNOW_KBID_DBG:
            return picokdbg_specializeDbgKnowledgeBase(*kb, this->common);
            break;
#endif

        default:
            break;
    }
    return PICO_OK;
}


static pico_status_t picorsrc_releaseKnowledgeBase(
        picorsrc_ResourceManager this,
        picoknow_KnowledgeBase * kb)
{
    (*kb) = NULL;
    return PICO_OK;
}

static pico_status_t picorsrc_getKbList(picorsrc_ResourceManager this,
        picoos_uint8 * data,
        picoos_uint32 datalen,
        picoknow_KnowledgeBase * kbList)
{

    pico_status_t status = PICO_OK;
    picoos_uint32 curpos = 0, offset, size;
    picoos_uint8 i, numKbs, kbid;
    picoos_char str[PICOKNOW_MAX_KB_NAME_SIZ];
    picoknow_KnowledgeBase kb;

    *kbList = NULL;
    datalen = datalen;
    /* read number of fields */
    numKbs = data[curpos++];
    PICODBG_DEBUG(("number of kbs (unrestricted) = %i",numKbs));
    status = (numKbs <= PICOKNOW_MAX_NUM_RESOURCE_KBS) ? PICO_OK : PICO_EXC_FILE_CORRUPT;
    /* read in all kb names */
    PICODBG_DEBUG(("number of kbs = %i",numKbs));
    i = 0;
    while ((PICO_OK == status) && (i++ < numKbs)) {
        status = (picoos_get_str((picoos_char *)data,&curpos,str,PICOOS_MAX_FIELD_STRING_LEN)) ? PICO_OK :  PICO_EXC_FILE_CORRUPT;
        PICODBG_DEBUG(("contains knowledge base %s (status: %i)",str, status));
    }
    /* consume termination of last str */
    curpos++;
    i = 0;
    while ((PICO_OK == status) && (i++ < numKbs)) {
        kbid = data[curpos++];
        PICODBG_DEBUG(("got kb id %i, curpos now %i",kbid, curpos));
        status = picoos_read_mem_pi_uint32(data,&curpos,&offset);
        PICODBG_DEBUG(("got kb offset %i, curpos now %i",offset, curpos));
        status = picoos_read_mem_pi_uint32(data,&curpos,&size);
        PICODBG_DEBUG(("got kb size %i, curpos now %i",size, curpos));
        if (PICO_OK == status) {
            if (0 == offset) {
                /* currently we consider a kb mentioned in resource but with offset 0 (no knowledge) as
                 * different form a kb not mentioned at all. We might reconsider that later. */
                PICODBG_DEBUG((" kb (id %i) is mentioned but empty (base:%i, size:%i)",kb->id, kb->base, kb->size));
                status = picorsrc_createKnowledgeBase(this, NULL, size, (picoknow_kb_id_t)kbid, &kb);
            } else {
                status = picorsrc_createKnowledgeBase(this, data+offset, size, (picoknow_kb_id_t)kbid, &kb);
            }
            PICODBG_DEBUG(("found kb (id %i) starting at %i with size %i",kb->id, kb->base, kb->size));
            if (PICO_OK == status) {
              kb->next = *kbList;
              *kbList = kb;
            }
        }
    }
    if (PICO_OK != status) {
        kb = *kbList;
        while (NULL != kb) {
            picorsrc_releaseKnowledgeBase(this,&kb);
        }
    }

    return status;

}

static pico_status_t picorsrc_releaseKbList(picorsrc_ResourceManager this, picoknow_KnowledgeBase * kbList)
{
    picoknow_KnowledgeBase kbprev, kb;
    kb = *kbList;
    while (NULL != kb) {
        kbprev = kb;
        kb = kb->next;
        picoknow_disposeKnowledgeBase(this->common->mm,&kbprev);
    }
    *kbList = NULL;
    return PICO_OK;
}


/* ******* accessing voice definitions **************************************/


static pico_status_t findVoiceDefinition(picorsrc_ResourceManager this,
        const picoos_char * voiceName, picorsrc_VoiceDefinition * vdef)
{
    picorsrc_VoiceDefinition v;
    PICODBG_DEBUG(("finding voice name %s",voiceName));
    if (NULL == this) {
        return PICO_ERR_NULLPTR_ACCESS;
    }
    v = this->vdefs;
    while (NULL != v && (0 != picoos_strcmp(v->voiceName,voiceName))) {
        PICODBG_DEBUG(("%s doesnt match",v->voiceName));
        v = v->next;
    }
    *vdef = v;
    if (v == NULL) {
        PICODBG_DEBUG(("didnt find voice name %s",voiceName));
    } else {
        PICODBG_DEBUG(("found voice name %s",voiceName));
   }
    return PICO_OK;
}

#ifdef __cplusplus
}
#endif

/* end picorsrc.c */
#endif