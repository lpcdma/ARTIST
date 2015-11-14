//
// Created by Lukas on 8/20/2015.
//

#ifndef NDKTEST_DEX_H
#define NDKTEST_DEX_H

#include <stddef.h>
#include <stdint.h>
#include "util.h"
#include "typedefs.h"

static const size_t kSha1DigestSize = 20;
static const uint32_t kDexEndianConstant = 0x12345678;

// The value of an invalid index.
static const uint32_t kDexNoIndex = 0xFFFFFFFF;

// The value of an invalid index.
static const uint16_t kDexNoIndex16 = 0xFFFF;

// The separator charactor in MultiDex locations.
static const char kMultiDexSeparator = ':';

// A string version of the previous. This is a define so that we can merge string literals in the
// preprocessor.
#define kMultiDexSeparatorString ":"

#define MemoryBackedValue(type, name) struct name { type* address; type value; };

typedef struct DexFileHeader
{
    uint8_t magic_[8];
    uint32_t checksum_;         // See also location_checksum_
    uint8_t signature_[20];    //[kSha1DigestSize];
    uint32_t file_size_;        // size of entire file
    uint32_t header_size_;      // offset to start of next section
    uint32_t endian_tag_;
    uint32_t link_size_;        // unused
    uint32_t link_off_;         // unused
    uint32_t map_off_;          // unused
    uint32_t string_ids_size_;  // number of StringIds
    uint32_t string_ids_off_;   // file offset of StringIds array
    uint32_t type_ids_size_;    // number of TypeIds, we don't support more than 65535
    uint32_t type_ids_off_;     // file offset of TypeIds array
    uint32_t proto_ids_size_;   // number of ProtoIds, we don't support more than 65535
    uint32_t proto_ids_off_;    // file offset of ProtoIds array
    uint32_t field_ids_size_;   // number of FieldIds
    uint32_t field_ids_off_;    // file offset of FieldIds array
    uint32_t method_ids_size_;  // number of MethodIds
    uint32_t method_ids_off_;   // file offset of MethodIds array
    uint32_t class_defs_size_;  // number of ClassDefs
    uint32_t class_defs_off_;   // file offset of ClassDef array
    uint32_t data_size_;        // unused
    uint32_t data_off_;         // unused
} DexFileHeader;

// Raw class_def_item.
typedef struct ClassDef
{
    uint16_t class_idx_;  // index into type_ids_ array for this class
    uint16_t pad1_;  // padding = 0
    uint32_t access_flags_;
    uint16_t superclass_idx_;  // index into type_ids_ array for superclass
    uint16_t pad2_;  // padding = 0
    uint32_t interfaces_off_;  // file offset to TypeList
    uint32_t source_file_idx_;  // index into string_ids_ for source file name
    uint32_t annotations_off_;  // file offset to annotations_directory_item
    uint32_t class_data_off_;  // file offset to class_data_item
    uint32_t static_values_off_;  // file offset to EncodedArray
} ClassDef;
typedef bool (*CLASSDEF_PREDICATE)(const DexFileHeader* hdr, ClassDef* c, void* args);

// Raw string_id_item.
typedef struct StringId
{
    uint32_t string_data_off_;  // offset in bytes from the base address
} StringId;
typedef bool (*STRINGID_PREDICATE)(const DexFileHeader* hdr, StringId* c, void* args);

// Raw method_id_item.
typedef struct MethodId
{
    uint16_t class_idx_;  // index into type_ids_ array for defining class
    uint16_t proto_idx_;  // index into proto_ids_ array for method prototype
    uint32_t name_idx_;  // index into string_ids_ array for method name
} MethodId;
typedef bool (*METHODID_PREDICATE)(const DexFileHeader* hdr, MethodId* c, void* args);

// Raw proto_id_item.
typedef struct ProtoId
{
    uint32_t shorty_idx_;       // index into string_ids array for shorty descriptor
    uint16_t return_type_idx_;  // index into type_ids array for return type
    uint16_t pad_;             // padding = 0
    uint32_t parameters_off_;  // file offset to type_list for parameter types
} ProtoId;
typedef bool (*PROTOID_PREDICATE)(const DexFileHeader* hdr, ProtoId* c, void* args);


// Raw type_id_item.
typedef struct TypeId
{
    uint32_t descriptor_idx_;  // index into string_ids
} TypeId;
typedef bool (*TYPEID_PREDICATE)(const DexFileHeader* hdr, TypeId* c, void* args);

// Raw field_id_item.
typedef struct FieldId
{
    uint16_t class_idx_;  // index into type_ids_ array for defining class
    uint16_t type_idx_;  // index into type_ids_ array for field type
    uint32_t name_idx_;  // index into string_ids_ array for field name
} FieldId;
typedef bool (*FIELDID_PREDICATE)(const DexFileHeader* hdr, FieldId* c, void* args);

// Raw code_item.
typedef struct CodeItem {
    uint16_t registers_size_;
    uint16_t ins_size_;
    uint16_t outs_size_;
    uint16_t tries_size_;
    uint32_t debug_info_off_;  // file offset to debug info stream
    uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
    uint16_t insns_[1];
} CodeItem;

/**
 * @param hdr       -   Pointer to the DexFileHeader at the beginning of the dex_file.
 * @param string_id -   Id of the string to get the contents of.
 *
 * @return  A String struct containing the length of the string, and a pointer to it's characters
 */
String      dex_file_GetStringDataByIndex(const DexFileHeader* hdr, uint32_t string_id);

String      dex_file_GetClassDefName(const DexFileHeader* hdr, uint32_t class_def_index);
String      dex_file_GetMethodIdName(const DexFileHeader* hdr, uint32_t method_id_index);
String      dex_file_GetTypeIdName(const DexFileHeader* hdr, uint32_t type_id_index);
String      dex_file_GetFieldIdName(const DexFileHeader* hdr, uint32_t field_id_index);

/**
 * @param hdr       -   Pointer to the DexFileHeader at the beginning of the dex_file.
 * @param original_class_def_index  -   Index into the class_def array of the original class
 *
 * @return  Index into the class_def array of the superclass of our original_class_def_index
 *          or kDexNoIndex if the class has no superclass.
 */
uint32_t    dex_file_FindSuperClassDefinitionIndex(const DexFileHeader* hdr, uint32_t original_class_def_index);
uint32_t    dex_file_FindClassDefinitionIndicesByPredicate(const DexFileHeader *hdr,
                                                           CLASSDEF_PREDICATE p, void *args,
                                                           uint32_t *result, uint32_t maxResults);
ClassDef    dex_file_GetClassDefinitionByIndex(const DexFileHeader* hdr, uint32_t class_def_index);
ProtoId     dex_file_GetMethodPrototypeByIndex(const DexFileHeader* hdr, uint32_t proto_id);
MethodId    dex_file_GetMethodDescriptorByIndex(const DexFileHeader* hdr, uint32_t method_id);
uint32_t    dex_file_FindMethodDescriptorIndicesByPredicate(const DexFileHeader* hdr, METHODID_PREDICATE p, void* args,
                                                         uint32_t* result, uint32_t maxResults);
TypeId      dex_file_GetTypeDescriptorByIndex(const DexFileHeader* hdr, uint32_t type_id);
FieldId     dex_file_GetFieldDescriptorByIndex(const DexFileHeader* hdr, uint32_t field_id);

/*
// Map item type codes.
enum {
    kDexTypeHeaderItem               = 0x0000,
    kDexTypeStringIdItem             = 0x0001,
    kDexTypeTypeIdItem               = 0x0002,
    kDexTypeProtoIdItem              = 0x0003,
    kDexTypeFieldIdItem              = 0x0004,
    kDexTypeMethodIdItem             = 0x0005,
    kDexTypeClassDefItem             = 0x0006,
    kDexTypeMapList                  = 0x1000,
    kDexTypeTypeList                 = 0x1001,
    kDexTypeAnnotationSetRefList     = 0x1002,
    kDexTypeAnnotationSetItem        = 0x1003,
    kDexTypeClassDataItem            = 0x2000,
    kDexTypeCodeItem                 = 0x2001,
    kDexTypeStringDataItem           = 0x2002,
    kDexTypeDebugInfoItem            = 0x2003,
    kDexTypeAnnotationItem           = 0x2004,
    kDexTypeEncodedArrayItem         = 0x2005,
    kDexTypeAnnotationsDirectoryItem = 0x2006,
};
struct MapItem {
    uint16_t type_;
    uint16_t unused_;
    uint32_t size_;
    uint32_t offset_;
};
struct MapList {
    uint32_t size_;
    struct MapItem list_[1];
};


// Raw type_id_item.
struct TypeId
{
    uint32_t descriptor_idx_;  // index into string_ids
};
// Raw field_id_item.
struct FieldId
{
    uint16_t class_idx_;  // index into type_ids_ array for defining class
    uint16_t type_idx_;  // index into type_ids_ array for field type
    uint32_t name_idx_;  // index into string_ids_ array for field name
};
// Raw method_id_item.
struct MethodId
{
    uint16_t class_idx_;  // index into type_ids_ array for defining class
    uint16_t proto_idx_;  // index into proto_ids_ array for method prototype
    uint32_t name_idx_;  // index into string_ids_ array for method name
};
// Raw proto_id_item.
struct ProtoId
{
    uint32_t shorty_idx_;  // index into string_ids array for shorty descriptor
    uint16_t return_type_idx_;  // index into type_ids array for return type
    uint16_t pad_;             // padding = 0
    uint32_t parameters_off_;  // file offset to type_list for parameter types
};

// Raw type_item.
struct TypeItem {
    uint16_t type_idx_;  // index into type_ids section
    private:
            DISALLOW_COPY_AND_ASSIGN(TypeItem);
};
// Raw type_list.
class TypeList {
    public:
    uint32_t Size() const {
        return size_;
    }
    const TypeItem& GetTypeItem(uint32_t idx) const {
        DCHECK_LT(idx, this->size_);
        return this->list_[idx];
    }
    // Size in bytes of the part of the list that is common.
    static constexpr size_t GetHeaderSize() {
        return 4U;
    }
    // Size in bytes of the whole type list including all the stored elements.
    static constexpr size_t GetListSize(size_t count) {
        return GetHeaderSize() + sizeof(TypeItem) * count;
    }
    private:
    uint32_t size_;  // size of the list, in entries
    TypeItem list_[1];  // elements of the list
    DISALLOW_COPY_AND_ASSIGN(TypeList);
};
// Raw code_item.
struct CodeItem {
    uint16_t registers_size_;
    uint16_t ins_size_;
    uint16_t outs_size_;
    uint16_t tries_size_;
    uint32_t debug_info_off_;  // file offset to debug info stream
    uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
    uint16_t insns_[1];
    private:
            DISALLOW_COPY_AND_ASSIGN(CodeItem);
};
// Raw try_item.
struct TryItem {
    uint32_t start_addr_;
    uint16_t insn_count_;
    uint16_t handler_off_;
    private:
            DISALLOW_COPY_AND_ASSIGN(TryItem);
};
// Annotation constants.
enum {
    kDexVisibilityBuild         = 0x00,     // annotation visibility
    kDexVisibilityRuntime       = 0x01,
    kDexVisibilitySystem        = 0x02,
    kDexAnnotationByte          = 0x00,
    kDexAnnotationShort         = 0x02,
    kDexAnnotationChar          = 0x03,
    kDexAnnotationInt           = 0x04,
    kDexAnnotationLong          = 0x06,
    kDexAnnotationFloat         = 0x10,
    kDexAnnotationDouble        = 0x11,
    kDexAnnotationString        = 0x17,
    kDexAnnotationType          = 0x18,
    kDexAnnotationField         = 0x19,
    kDexAnnotationMethod        = 0x1a,
    kDexAnnotationEnum          = 0x1b,
    kDexAnnotationArray         = 0x1c,
    kDexAnnotationAnnotation    = 0x1d,
    kDexAnnotationNull          = 0x1e,
    kDexAnnotationBoolean       = 0x1f,
    kDexAnnotationValueTypeMask = 0x1f,     // low 5 bits
     kDexAnnotationValueArgShift = 5,
};
struct AnnotationsDirectoryItem {
    uint32_t class_annotations_off_;
    uint32_t fields_size_;
    uint32_t methods_size_;
    uint32_t parameters_size_;
    private:
            DISALLOW_COPY_AND_ASSIGN(AnnotationsDirectoryItem);
};
struct FieldAnnotationsItem {
    uint32_t field_idx_;
    uint32_t annotations_off_;
    private:
            DISALLOW_COPY_AND_ASSIGN(FieldAnnotationsItem);
};
struct MethodAnnotationsItem {
    uint32_t method_idx_;
    uint32_t annotations_off_;
    private:
            DISALLOW_COPY_AND_ASSIGN(MethodAnnotationsItem);
};
struct ParameterAnnotationsItem {
    uint32_t method_idx_;
    uint32_t annotations_off_;
    private:
            DISALLOW_COPY_AND_ASSIGN(ParameterAnnotationsItem);
};
struct AnnotationSetRefItem {
    uint32_t annotations_off_;
    private:
            DISALLOW_COPY_AND_ASSIGN(AnnotationSetRefItem);
};
struct AnnotationSetRefList {
    uint32_t size_;
    AnnotationSetRefItem list_[1];
    private:
            DISALLOW_COPY_AND_ASSIGN(AnnotationSetRefList);
};
struct AnnotationSetItem {
    uint32_t size_;
    uint32_t entries_[1];
    private:
            DISALLOW_COPY_AND_ASSIGN(AnnotationSetItem);
};
struct AnnotationItem {
    uint8_t visibility_;
    uint8_t annotation_[1];
    private:
            DISALLOW_COPY_AND_ASSIGN(AnnotationItem);
};
 */


#endif //NDKTEST_DEX_FILE_H
