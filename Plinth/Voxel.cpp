// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Voxel.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

#include "Common.hpp"

////////////////////////////////////////////////////////////

voxel_grid::voxel_grid(std::vector<voxel> const& voxels)
{
    for (auto const& v : voxels) {
        if (v.Position.X < 0 || v.Position.Y < 0 || v.Position.Z < 0) { continue; }

        Size.X = std::max(Size.X, v.Position.X + 1);
        Size.Y = std::max(Size.Y, v.Position.Y + 1);
        Size.Z = std::max(Size.Z, v.Position.Z + 1);
    }

    // an empty cell is a transparent one, a voxel is always opaque
    isize const cellCount {static_cast<isize>(Size.X) * Size.Y * Size.Z};
    _cells.assign(static_cast<usize>(std::max<isize>(cellCount, 0)), colors::Transparent);

    for (auto const& v : voxels) {
        if (v.Position.X < 0 || v.Position.Y < 0 || v.Position.Z < 0) { continue; }

        color c {v.Color};
        c.A                                                                              = 255;
        _cells[static_cast<usize>(flat_index(v.Position.X, v.Position.Y, v.Position.Z))] = c;
    }
}

auto voxel_grid::flat_index(i32 x, i32 y, i32 z) const -> isize
{
    return x + (static_cast<isize>(y) * Size.X) + (static_cast<isize>(z) * Size.X * Size.Y);
}

auto voxel_grid::occupied(i32 x, i32 y, i32 z) const -> bool
{
    if (x < 0 || y < 0 || z < 0 || x >= Size.X || y >= Size.Y || z >= Size.Z) { return false; }
    return _cells[static_cast<usize>(flat_index(x, y, z))].A != 0;
}

auto voxel_grid::color_at(i32 x, i32 y, i32 z) const -> color
{
    return _cells[static_cast<usize>(flat_index(x, y, z))];
}

auto voxel_grid::raycast(point3_d const& origin, point3_d const& dir, f64 maxT) const -> voxel_ray_hit
{
    voxel_ray_hit result {};
    if (Size.X <= 0 || Size.Y <= 0 || Size.Z <= 0) { return result; }

    std::array<f64, 3> const o {origin.X, origin.Y, origin.Z};
    std::array<f64, 3> const d {dir.X, dir.Y, dir.Z};
    std::array<i32, 3> const size {Size.X, Size.Y, Size.Z};

    // clip the ray to the box; the face it comes in through is the one of the axis that enters last
    f64 tMin {0.0};
    f64 tMax {maxT};
    i32 entryAxis {2};
    i32 entrySign {1};

    for (i32 axis {0}; axis < 3; ++axis) {
        f64 const hi {static_cast<f64>(size[axis])};
        if (std::abs(d[axis]) < 1e-12) {
            if (o[axis] < 0.0 || o[axis] > hi) { return result; }
            continue;
        }

        f64 t0 {(0.0 - o[axis]) / d[axis]};
        f64 t1 {(hi - o[axis]) / d[axis]};
        i32 sign {-1};
        if (t0 > t1) {
            std::swap(t0, t1);
            sign = 1;
        }
        if (t0 > tMin) {
            tMin      = t0;
            entryAxis = axis;
            entrySign = sign;
        }
        tMax = std::min(tMax, t1);
        if (tMin > tMax) { return result; }
    }

    f64 const                entryT {std::max(tMin, 0.0)};
    point3_d const           start {origin + (dir * entryT)};
    std::array<f64, 3> const first {start.X, start.Y, start.Z};

    std::array<i32, 3> voxelPos {};
    std::array<i32, 3> step {};
    std::array<f64, 3> tNext {};
    std::array<f64, 3> tDelta {};
    for (i32 axis {0}; axis < 3; ++axis) {
        constexpr f64 INF {std::numeric_limits<f64>::infinity()};

        voxelPos[axis] = std::clamp(static_cast<i32>(std::floor(first[axis])), 0, size[axis] - 1);
        step[axis]     = d[axis] > 0 ? 1 : (d[axis] < 0 ? -1 : 0);
        tNext[axis]    = step[axis] == 0 ? INF : ((step[axis] > 0 ? voxelPos[axis] + 1 : voxelPos[axis]) - o[axis]) / d[axis];
        tDelta[axis]   = d[axis] == 0.0 ? INF : std::abs(1.0 / d[axis]);
    }

    f64 currentT {entryT};
    i32 currentAxis {entryAxis};
    i32 currentSign {entrySign};

    for (;;) {
        if (occupied(voxelPos[0], voxelPos[1], voxelPos[2])) {
            result.Hit      = true;
            result.T        = currentT;
            result.Color    = color_at(voxelPos[0], voxelPos[1], voxelPos[2]);
            result.FaceAxis = currentAxis;
            result.FaceSign = currentSign;
            result.Cell     = vec3_i {.X = voxelPos[0], .Y = voxelPos[1], .Z = voxelPos[2]};
            return result;
        }

        // on to the voxel behind the nearest boundary
        i32 axis {2};
        if (tNext[0] < tNext[1] && tNext[0] < tNext[2]) {
            axis = 0;
        } else if (tNext[1] < tNext[2]) {
            axis = 1;
        }

        voxelPos[axis] += step[axis];
        currentT = tNext[axis];
        tNext[axis] += tDelta[axis];
        currentAxis = axis;
        currentSign = -step[axis];

        if (currentT > tMax || voxelPos[axis] < 0 || voxelPos[axis] >= size[axis]) { break; }
    }

    return result;
}

auto voxel_grid::pivot() const -> point3_d { return {.X = Size.X * 0.5, .Y = Size.Y * 0.5, .Z = Size.Z * 0.5}; }

auto voxel_grid::diagonal() const -> f64 { return std::sqrt(static_cast<f64>((Size.X * Size.X) + (Size.Y * Size.Y) + (Size.Z * Size.Z))); }

auto voxel_grid::corners() const -> std::array<point3_d, 8>
{
    f64 const sx {static_cast<f64>(Size.X)}, sy {static_cast<f64>(Size.Y)}, sz {static_cast<f64>(Size.Z)};
    return {{{.X = 0.0, .Y = 0.0, .Z = 0.0}, {.X = sx, .Y = 0.0, .Z = 0.0}, {.X = 0.0, .Y = sy, .Z = 0.0}, {.X = sx, .Y = sy, .Z = 0.0}, {.X = 0.0, .Y = 0.0, .Z = sz}, {.X = sx, .Y = 0.0, .Z = sz}, {.X = 0.0, .Y = sy, .Z = sz}, {.X = sx, .Y = sy, .Z = sz}}};
}

// the two axes that span a face, by the axis of its normal
constexpr static std::array<std::array<i32, 2>, 3> FACE_AXES {{{1, 2}, {0, 2}, {0, 1}}};

auto voxel_grid::face_ao(voxel_ray_hit const& hit, point3_d const& localHitPos) const -> f64
{
    auto const [ua, va] {FACE_AXES[static_cast<usize>(hit.FaceAxis)]};

    std::array<i32, 3> const cell {hit.Cell.X, hit.Cell.Y, hit.Cell.Z};
    std::array<f64, 3> const local {localHitPos.X, localHitPos.Y, localHitPos.Z};
    std::array<i32, 3>       layer {cell}; // the layer of voxels in front of the face
    layer[static_cast<usize>(hit.FaceAxis)] += hit.FaceSign;

    i32 const ao00 {corner_ao(layer, hit.FaceAxis, -1, -1)};
    i32 const ao10 {corner_ao(layer, hit.FaceAxis, +1, -1)};
    i32 const ao01 {corner_ao(layer, hit.FaceAxis, -1, +1)};
    i32 const ao11 {corner_ao(layer, hit.FaceAxis, +1, +1)};

    f64 const fu {std::clamp(local[static_cast<usize>(ua)] - cell[static_cast<usize>(ua)], 0.0, 1.0)};
    f64 const fv {std::clamp(local[static_cast<usize>(va)] - cell[static_cast<usize>(va)], 0.0, 1.0)};

    f64 const aoInterp {(ao00 * (1.0 - fu) * (1.0 - fv)) + (ao10 * fu * (1.0 - fv)) + (ao01 * (1.0 - fu) * fv) + (ao11 * fu * fv)};
    return aoInterp / 3.0;
}

auto voxel_grid::corner_ao(std::array<i32, 3> const& layer, i32 faceAxis, i32 cu, i32 cv) const -> i32
{
    auto const [ua, va] {FACE_AXES[static_cast<usize>(faceAxis)]};

    // whether the voxel in front of the face, shifted by du and dv along the face, is filled
    auto const filled {[&](i32 du, i32 dv) {
        auto p {layer};
        p[static_cast<usize>(ua)] += du;
        p[static_cast<usize>(va)] += dv;
        return occupied(p[0], p[1], p[2]);
    }};

    bool const side1 {filled(cu, 0)};
    bool const side2 {filled(0, cv)};
    bool const corner {filled(cu, cv)};

    if (side1 && side2) { return 0; }
    return 3 - (static_cast<i32>(side1) + static_cast<i32>(side2) + static_cast<i32>(corner));
}

////////////////////////////////////////////////////////////

auto voxel_transform::to_world(point3_d local) const -> point3_d // voxel space -> world
{
    point3_d const r {Mat3Rotate(Rotation, {.X = (local.X - Pivot.X) * Scale, .Y = (local.Y - Pivot.Y) * Scale, .Z = (local.Z - Pivot.Z) * Scale})};
    return {.X = Position.X + r.X, .Y = Position.Y + r.Y, .Z = BaseZ + (Pivot.Z * Scale) + r.Z};
}

auto voxel_transform::to_local(point3_d world) const -> point3_d
{
    point3_d const r {Mat3RotateInverse(Rotation, {.X = world.X - Position.X, .Y = world.Y - Position.Y, .Z = world.Z - (BaseZ + (Pivot.Z * Scale))})};
    return {.X = (r.X / Scale) + Pivot.X, .Y = (r.Y / Scale) + Pivot.Y, .Z = (r.Z / Scale) + Pivot.Z};
}

auto voxel_transform::dir_to_local(point3_d worldDir) const -> point3_d // keeps ray parameters in world units
{
    point3_d const r {Mat3RotateInverse(Rotation, worldDir)};
    return {.X = r.X / Scale, .Y = r.Y / Scale, .Z = r.Z / Scale};
}

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

////////////////////////////////////////////////////////////

auto LoadVoxels(string const& path) -> std::optional<voxel_grid>
{
    io::ifstream stream {path};
    struct raw_voxel {
        u8 X, Y, Z;
        u8 ColorIndex;
    };
    std::vector<raw_voxel> rawVoxels;

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
            rawVoxels.reserve(numVoxels);
            for (u32 i {0}; i < numVoxels; ++i) {
                u8 const x {stream.read<u8>()};
                u8 const y {stream.read<u8>()};
                u8 const z {stream.read<u8>()};
                u8 const colorIndex {stream.read<u8>()};
                if (x >= size.X || y >= size.Y || z >= size.Z) { continue; }
                rawVoxels.push_back({.X = x, .Y = y, .Z = z, .ColorIndex = colorIndex});
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

    std::vector<voxel> result;
    result.reserve(rawVoxels.size());
    for (auto const& rv : rawVoxels) {
        result.push_back(voxel {.Position = {.X = rv.X, .Y = rv.Y, .Z = rv.Z}, .Color = color::FromABGR(palette[rv.ColorIndex])});
    }

    return voxel_grid {result};
}
