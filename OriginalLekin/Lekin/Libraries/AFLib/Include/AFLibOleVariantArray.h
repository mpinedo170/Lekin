#pragma once

/////////////////////////////////////////////////////////////////////////////
// COleVariantArray
// Array of variants: separate include file
//   because user code may not include COleVariant

namespace AFLib {
class COleVariantArray : public CArray<COleVariant>
{};
}  // namespace AFLib
