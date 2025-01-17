// stardb.h
//
// Copyright (C) 2001-2009, the Celestia Development Team
// Original version by Chris Laurel <claurel@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <celcompat/filesystem.h>
#include <celengine/parseobject.h>
#include <celutil/blockarray.h>
#include "astroobj.h"
#include "hash.h"
#include "staroctree.h"


class StarNameDatabase;


constexpr inline unsigned int MAX_STAR_NAMES = 10;

class StarDatabase
{
 public:
    StarDatabase();
    ~StarDatabase();

    inline Star* getStar(const std::uint32_t) const;
    inline std::uint32_t size() const;

    Star* find(AstroCatalog::IndexNumber catalogNumber) const;
    Star* find(std::string_view, bool i18n) const;
    AstroCatalog::IndexNumber findCatalogNumberByName(std::string_view, bool i18n) const;

    void getCompletion(std::vector<std::string>&, std::string_view, bool i18n) const;

    void findVisibleStars(StarHandler& starHandler,
                          const Eigen::Vector3f& obsPosition,
                          const Eigen::Quaternionf&   obsOrientation,
                          float fovY,
                          float aspectRatio,
                          float limitingMag,
                          OctreeProcStats * = nullptr) const;

    void findCloseStars(StarHandler& starHandler,
                        const Eigen::Vector3f& obsPosition,
                        float radius) const;

    std::string getStarName(const Star&, bool i18n = false) const;
    std::string getStarNameList(const Star&, const unsigned int maxNames = MAX_STAR_NAMES) const;

    StarNameDatabase* getNameDatabase() const;
    void setNameDatabase(StarNameDatabase*);

    bool load(std::istream&, const fs::path& resourcePath = fs::path());
    bool loadBinary(std::istream&);

    enum Catalog
    {
        HenryDraper = 0,
        Gliese      = 1,
        SAO         = 2,
        MaxCatalog  = 3,
    };

    // Not exact, but any star with a catalog number greater than this is assumed to not be
    // a HIPPARCOS stars.
    static constexpr AstroCatalog::IndexNumber MAX_HIPPARCOS_NUMBER = 999999;

    struct CrossIndexEntry
    {
        AstroCatalog::IndexNumber catalogNumber;
        AstroCatalog::IndexNumber celCatalogNumber;

        bool operator<(const CrossIndexEntry&) const;
    };

    using CrossIndex = std::vector<CrossIndexEntry>;

    bool loadCrossIndex(const Catalog, std::istream&);
    AstroCatalog::IndexNumber searchCrossIndexForCatalogNumber(const Catalog, const AstroCatalog::IndexNumber number) const;
    Star* searchCrossIndex(const Catalog, const AstroCatalog::IndexNumber number) const;
    AstroCatalog::IndexNumber crossIndex(const Catalog, const AstroCatalog::IndexNumber number) const;

    void finish();

    struct CustomStarDetails;

private:
    bool createStar(Star* star,
                    DataDisposition disposition,
                    AstroCatalog::IndexNumber catalogNumber,
                    const Hash* starData,
                    const fs::path& path,
                    const bool isBarycenter);
    bool createOrUpdateStarDetails(Star* star,
                                   DataDisposition disposition,
                                   AstroCatalog::IndexNumber catalogNumber,
                                   const Hash* starData,
                                   const fs::path& path,
                                   const bool isBarycenter,
                                   std::optional<Eigen::Vector3f>& barycenterPosition);
    bool applyCustomStarDetails(const Star*,
                                AstroCatalog::IndexNumber,
                                const Hash*,
                                const fs::path&,
                                const CustomStarDetails&,
                                std::optional<Eigen::Vector3f>&);
    bool applyOrbit(AstroCatalog::IndexNumber catalogNumber,
                    const Hash* starData,
                    StarDetails* details,
                    const CustomStarDetails& customDetails,
                    std::optional<Eigen::Vector3f>& barycenterPosition);

    void buildOctree();
    void buildIndexes();
    Star* findWhileLoading(AstroCatalog::IndexNumber catalogNumber) const;

    std::uint32_t nStars{ 0 };

    Star*             stars{ nullptr };
    StarNameDatabase* namesDB{ nullptr };
    Star**            catalogNumberIndex{ nullptr };
    StarOctree*       octreeRoot{ nullptr };
    AstroCatalog::IndexNumber nextAutoCatalogNumber{ 0xfffffffe };

    std::vector<CrossIndex*> crossIndexes;

    // These values are used by the star database loader; they are
    // not used after loading is complete.
    BlockArray<Star> unsortedStars;
    // List of stars loaded from binary file, sorted by catalog number
    Star** binFileCatalogNumberIndex{ nullptr };
    unsigned int binFileStarCount{ 0 };
    // Catalog number -> star mapping for stars loaded from stc files
    std::map<AstroCatalog::IndexNumber, Star*> stcFileCatalogNumberIndex;

    struct BarycenterUsage
    {
        AstroCatalog::IndexNumber catNo;
        AstroCatalog::IndexNumber barycenterCatNo;
    };
    std::vector<BarycenterUsage> barycenters;
};


Star* StarDatabase::getStar(const std::uint32_t n) const
{
    return stars + n;
}

std::uint32_t StarDatabase::size() const
{
    return nStars;
}
