// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Voxel.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

////////////////////////////////////////////////////////////

constexpr std::array<u32, 256> kDefaultPalette {
    0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
    0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
    0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
    0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
    0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
    0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
    0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
    0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
    0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
    0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
    0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
    0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
    0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
    0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
    0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
    0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111};

constexpr std::array<char, 3> SIGNATURE {'V', 'O', 'X'};

auto load_vox_bytes(io::istream& stream) -> std::optional<voxel_grid>
{
    std::vector<voxel> result;
    if (stream.size_in_bytes() < 20) { return std::nullopt; }
    std::array<char, 3> magic {};
    stream.read_to<char>(magic);
    if (magic != SIGNATURE) { return std::nullopt; }

    stream.seek(8, io::seek_dir::Begin);

    std::array<u32, 256> palette {kDefaultPalette};
    bool                 haveSize {false};
    bool                 haveXyzi {false};
    vec3_i               size {};

    while (!stream.is_eof()) {
        string id {stream.read_string(4)};

        u32 const   contentSize {stream.read<u32>()};
        u32 const   childrenSize {stream.read<u32>()};
        isize const chunkEnd {stream.tell() + contentSize};

        if (id == "MAIN") {
            stream.seek(chunkEnd, io::seek_dir::Begin);
            continue;
        }

        if (id == "SIZE") {
            size.X   = stream.read<i32>();
            size.Y   = stream.read<i32>();
            size.Z   = stream.read<i32>();
            haveSize = true;
        } else if (id == "XYZI" && haveSize && !haveXyzi) {
            u32 const numVoxels {stream.read<u32>()};
            result.reserve(numVoxels);
            for (u32 i {0}; i < numVoxels; ++i) {
                u8 const x {stream.read<u8>()};
                u8 const y {stream.read<u8>()};
                u8 const z {stream.read<u8>()};
                u8 const colorIndex {stream.read<u8>()};
                if (x >= size.X || y >= size.Y || z >= size.Z) { continue; } // malformed entry, skip
                result.push_back(voxel {.Position = {.X = x, .Y = y, .Z = z}, .Color = color::FromABGR(palette[colorIndex])});
            }
            haveXyzi = true;
        } else if (id == "RGBA") {
            for (i32 i {0}; i < 255; ++i) {
                palette[i + 1] = stream.read<u32>();
            }
        }

        stream.seek(chunkEnd + childrenSize, io::seek_dir::Begin);
    }

    if (!haveSize || !haveXyzi) { return std::nullopt; }
    return {result};
}

auto load_vox_file(std::string const& path) -> std::optional<voxel_grid>
{
    io::ifstream file {path};
    return load_vox_bytes(file);
}

////////////////////////////////////////////////////////////

struct voxel_ray_hit {
    bool   Hit {false};
    f64    T {0.0};
    color  Color {};
    i32    FaceAxis {0};
    i32    FaceSign {1};
    vec3_i Cell {};
};

static auto key(i32 x, i32 y, i32 z) -> i64 { return static_cast<i64>(x) | (static_cast<i64>(y) << 20) | (static_cast<i64>(z) << 40); }

voxel_grid::voxel_grid(std::vector<voxel> const& voxels)
{
    for (voxel const& v : voxels) {
        if (v.Position.X < 0 || v.Position.Y < 0 || v.Position.Z < 0) { continue; }

        Size.X = std::max(Size.X, v.Position.X + 1);
        Size.Y = std::max(Size.Y, v.Position.Y + 1);
        Size.Z = std::max(Size.Z, v.Position.Z + 1);
        Cells.insert_or_assign(key(v.Position.X, v.Position.Y, v.Position.Z), v.Color);
    }
}

static auto raycast_voxel_grid(voxel_grid const& grid, vec3_d const& origin, vec3_d const& dir, f64 maxT) -> voxel_ray_hit
{
    voxel_ray_hit result {};
    if (grid.Cells.empty()) { return result; }

    f64 tMin {0.0};
    f64 tMax {maxT};

    i32 lastAxis {2};
    i32 lastSign {1};

    auto const clip_axis {[&](f64 o, f64 d, f64 lo, f64 hi, i32 axis) -> bool {
        if (std::abs(d) < 1e-12) { return o >= lo && o <= hi; }
        f64 t0 {(lo - o) / d};
        f64 t1 {(hi - o) / d};
        i32 s0 {-1};
        i32 s1 {1};
        if (t0 > t1) {
            std::swap(t0, t1);
            std::swap(s0, s1);
        }
        if (t0 > tMin) {
            tMin     = t0;
            lastAxis = axis;
            lastSign = s0;
        }
        tMax = std::min(tMax, t1);
        return tMin <= tMax;
    }};

    if (!clip_axis(origin.X, dir.X, 0.0, static_cast<f64>(grid.Size.X), 0)) { return result; }
    if (!clip_axis(origin.Y, dir.Y, 0.0, static_cast<f64>(grid.Size.Y), 1)) { return result; }
    if (!clip_axis(origin.Z, dir.Z, 0.0, static_cast<f64>(grid.Size.Z), 2)) { return result; }
    if (tMin > tMax) { return result; }

    vec3_d const start {origin + (dir * std::max(tMin, 0.0))};

    i32 voxelX {std::clamp(static_cast<i32>(std::floor(start.X)), 0, grid.Size.X - 1)};
    i32 voxelY {std::clamp(static_cast<i32>(std::floor(start.Y)), 0, grid.Size.Y - 1)};
    i32 voxelZ {std::clamp(static_cast<i32>(std::floor(start.Z)), 0, grid.Size.Z - 1)};

    i32 const stepX {dir.X > 0 ? 1 : (dir.X < 0 ? -1 : 0)};
    i32 const stepY {dir.Y > 0 ? 1 : (dir.Y < 0 ? -1 : 0)};
    i32 const stepZ {dir.Z > 0 ? 1 : (dir.Z < 0 ? -1 : 0)};

    auto const next_boundary_t {[&](f64 o, f64 d, i32 voxel, i32 step) -> f64 {
        if (step == 0) { return std::numeric_limits<f64>::infinity(); }
        f64 const boundary {step > 0 ? static_cast<f64>(voxel + 1) : static_cast<f64>(voxel)};
        return (boundary - o) / d;
    }};
    auto const t_delta {[&](f64 d) -> f64 {
        return d == 0.0 ? std::numeric_limits<f64>::infinity() : std::abs(1.0 / d);
    }};

    f64       tMaxX {next_boundary_t(origin.X, dir.X, voxelX, stepX)};
    f64       tMaxY {next_boundary_t(origin.Y, dir.Y, voxelY, stepY)};
    f64       tMaxZ {next_boundary_t(origin.Z, dir.Z, voxelZ, stepZ)};
    f64 const tDeltaX {t_delta(dir.X)};
    f64 const tDeltaY {t_delta(dir.Y)};
    f64 const tDeltaZ {t_delta(dir.Z)};

    for (;;) {
        if (voxelX < 0 || voxelX >= grid.Size.X || voxelY < 0 || voxelY >= grid.Size.Y || voxelZ < 0 || voxelZ >= grid.Size.Z) { break; }

        if (auto const it {grid.Cells.find(key(voxelX, voxelY, voxelZ))}; it != grid.Cells.end()) {
            result.Hit      = true;
            result.T        = std::max(tMin, 0.0);
            result.Color    = it->second;
            result.FaceAxis = lastAxis;
            result.FaceSign = lastSign;
            result.Cell     = vec3_i {.X = voxelX, .Y = voxelY, .Z = voxelZ};
            return result;
        }

        if (tMaxX < tMaxY && tMaxX < tMaxZ) {
            voxelX += stepX;
            tMin = tMaxX;
            tMaxX += tDeltaX;
            lastAxis = 0;
            lastSign = -stepX;
        } else if (tMaxY < tMaxZ) {
            voxelY += stepY;
            tMin = tMaxY;
            tMaxY += tDeltaY;
            lastAxis = 1;
            lastSign = -stepY;
        } else {
            voxelZ += stepZ;
            tMin = tMaxZ;
            tMaxZ += tDeltaZ;
            lastAxis = 2;
            lastSign = -stepZ;
        }

        if (tMin > tMax) { break; }
    }

    return result;
}

static auto light_from_front(f64 frontFacingDegrees, f64 azimuthOffsetDeg, f64 elevationDeg) -> vec3_d
{
    f64 const azimuthRad {(frontFacingDegrees + azimuthOffsetDeg) * (TAU / 360.0)};
    f64 const elevRad {elevationDeg * (TAU / 360.0)};
    return vec3_d {
        .X = std::cos(azimuthRad) * std::cos(elevRad),
        .Y = std::sin(azimuthRad) * std::cos(elevRad),
        .Z = std::sin(elevRad),
    }
        .normalized();
}

static auto face_normal(i32 axis, i32 sign) -> vec3_d
{
    vec3_d    n {};
    f64 const s {static_cast<f64>(sign)};
    switch (axis) {
    case 0:  n.X = s; break;
    case 1:  n.Y = s; break;
    default: n.Z = s; break;
    }

    return n;
}

static auto corner_ao(voxel_grid const& grid, vec3_i layer, i32 faceAxis, i32 cu, i32 cv) -> i32
{
    auto const occ {[&](i32 x, i32 y, i32 z) { return grid.Cells.contains(key(x, y, z)); }};

    bool side1 {false};
    bool side2 {false};
    bool corner {false};
    switch (faceAxis) {
    case 0:
        side1  = occ(layer.X, layer.Y + cu, layer.Z);
        side2  = occ(layer.X, layer.Y, layer.Z + cv);
        corner = occ(layer.X, layer.Y + cu, layer.Z + cv);
        break;
    case 1:
        side1  = occ(layer.X + cu, layer.Y, layer.Z);
        side2  = occ(layer.X, layer.Y, layer.Z + cv);
        corner = occ(layer.X + cu, layer.Y, layer.Z + cv);
        break;
    default:
        side1  = occ(layer.X + cu, layer.Y, layer.Z);
        side2  = occ(layer.X, layer.Y + cv, layer.Z);
        corner = occ(layer.X + cu, layer.Y + cv, layer.Z);
        break;
    }

    if (side1 && side2) { return 0; }
    return 3 - (static_cast<i32>(side1) + static_cast<i32>(side2) + static_cast<i32>(corner));
}

auto voxel_grid::bake_facings(i32 frameSize, f64 frontFacingDegrees, i32 numFacings, bake_lighting const& lighting) const -> std::vector<u8>
{
    std::vector<u8> retValue {};
    retValue.resize(static_cast<usize>(numFacings * frameSize * frameSize * TEXTURE_BPP));

    f64 const extentVert {static_cast<f64>(Size.Z)};

    f64 const extentHoriz {extentVert};

    f64 const footprintDiag {std::sqrt(static_cast<f64>((Size.X * Size.X) + (Size.Y * Size.Y)))};
    f64 const cameraDist {footprintDiag + extentVert + 4.0};
    f64 const maxT {(cameraDist * 2.0) + extentVert};

    vec3_d const centerLocal {.X = Size.X * 0.5, .Y = Size.Y * 0.5, .Z = extentVert * 0.5};

    vec3_d const keyLight {light_from_front(frontFacingDegrees, lighting.KeyAzimuthOffsetDeg, lighting.KeyElevationDeg)};
    vec3_d const fillLight {light_from_front(frontFacingDegrees, lighting.FillAzimuthOffsetDeg, lighting.FillElevationDeg)};

    for (i32 f {0}; f < numFacings; ++f) {
        f64 const thetaDeg {frontFacingDegrees + (static_cast<f64>(f) * (360.0 / numFacings))};
        f64 const theta {thetaDeg * (TAU / 360.0)};

        vec3_d const camDir {.X = std::cos(theta), .Y = std::sin(theta), .Z = 0.0};
        vec3_d const right {.X = -std::sin(theta), .Y = std::cos(theta), .Z = 0.0};
        vec3_d const up {.X = 0.0, .Y = 0.0, .Z = 1.0};
        vec3_d const rayDir {.X = -camDir.X, .Y = -camDir.Y, .Z = -camDir.Z};

        auto* frameBuf {retValue.data() + (f * frameSize * frameSize * TEXTURE_BPP)};

        for (i32 py {0}; py < frameSize; ++py) {
            f64 const v {(0.5 - ((py + 0.5) / frameSize)) * extentVert};

            for (i32 px {0}; px < frameSize; ++px) {
                f64 const u {(((px + 0.5) / frameSize) - 0.5) * extentHoriz};

                vec3_d const rayOrigin {
                    .X = centerLocal.X + (camDir.X * cameraDist) + (right.X * u) + (up.X * v),
                    .Y = centerLocal.Y + (camDir.Y * cameraDist) + (right.Y * u) + (up.Y * v),
                    .Z = centerLocal.Z + (camDir.Z * cameraDist) + (right.Z * u) + (up.Z * v),
                };

                auto const hit {raycast_voxel_grid(*this, rayOrigin, rayDir, maxT)};

                i32 const idx {((py * frameSize) + px) * TEXTURE_BPP};
                if (!hit.Hit) {
                    frameBuf[idx + 0] = 0x98;
                    frameBuf[idx + 1] = 0x00;
                    frameBuf[idx + 2] = 0x88;
                    continue;
                }

                vec3_d const normal {face_normal(hit.FaceAxis, hit.FaceSign)};
                f64 const    keyTerm {lighting.KeyDiffuse * std::max(0.0, normal.dot(keyLight))};
                f64 const    fillTerm {lighting.FillDiffuse * std::max(0.0, normal.dot(fillLight))};
                f64 const    ambientTerm {lighting.AmbientGround + ((lighting.AmbientSky - lighting.AmbientGround) * ((normal.Z * 0.5) + 0.5))};
                f64 const    heightFactor {static_cast<f64>(hit.Cell.Z) / static_cast<f64>(std::max(1, Size.Z - 1))};
                f64 const    heightMultiplier {1.0 - lighting.HeightBandingStrength + (lighting.HeightBandingStrength * heightFactor)};

                vec3_i const layer {
                    .X = hit.Cell.X + (hit.FaceAxis == 0 ? hit.FaceSign : 0),
                    .Y = hit.Cell.Y + (hit.FaceAxis == 1 ? hit.FaceSign : 0),
                    .Z = hit.Cell.Z + (hit.FaceAxis == 2 ? hit.FaceSign : 0),
                };
                i32 const ao00 {corner_ao(*this, layer, hit.FaceAxis, -1, -1)};
                i32 const ao10 {corner_ao(*this, layer, hit.FaceAxis, +1, -1)};
                i32 const ao01 {corner_ao(*this, layer, hit.FaceAxis, -1, +1)};
                i32 const ao11 {corner_ao(*this, layer, hit.FaceAxis, +1, +1)};

                vec3_d const hitPos {rayOrigin + (rayDir * hit.T)};
                f64          fu {}, fv {};
                switch (hit.FaceAxis) {
                case 0:
                    fu = hitPos.Y - hit.Cell.Y;
                    fv = hitPos.Z - hit.Cell.Z;
                    break;
                case 1:
                    fu = hitPos.X - hit.Cell.X;
                    fv = hitPos.Z - hit.Cell.Z;
                    break;
                default:
                    fu = hitPos.X - hit.Cell.X;
                    fv = hitPos.Y - hit.Cell.Y;
                    break;
                }
                fu = std::clamp(fu, 0.0, 1.0);
                fv = std::clamp(fv, 0.0, 1.0);

                f64 const aoInterp {(ao00 * (1.0 - fu) * (1.0 - fv)) + (ao10 * fu * (1.0 - fv)) + (ao01 * (1.0 - fu) * fv) + (ao11 * fu * fv)};
                f64 const aoFactor {1.0 - (lighting.AoStrength * (1.0 - (aoInterp / 3.0)))};

                f64 const shade {std::clamp((ambientTerm + keyTerm + fillTerm) * aoFactor * heightMultiplier, 0.0, 1.0)};

                frameBuf[idx + 0] = static_cast<u8>(std::min(255.0, hit.Color.R * shade));
                frameBuf[idx + 1] = static_cast<u8>(std::min(255.0, hit.Color.G * shade));
                frameBuf[idx + 2] = static_cast<u8>(std::min(255.0, hit.Color.B * shade));
            }
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

auto vec3_d::dot(vec3_d const& b) const -> f64 { return (X * b.X) + (Y * b.Y) + (Z * b.Z); }

auto vec3_d::normalized() const -> vec3_d
{
    f64 const len {std::sqrt(dot(*this))};
    return len > 1e-12 ? *this * (1.0 / len) : vec3_d {.X = 0.0, .Y = 0.0, .Z = 1.0};
}
