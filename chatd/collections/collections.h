#ifndef COLLECTIONS_H
#define COLLECTIONS_H

/**
 * enum cresult - Return codes for collection operations
 * @C_OK: Operation succeeded
 * @C_NOMEM: Memory allocation failed
 * @C_EMPTY: Collection is empty
 * @C_OUT_OF_BOUNDS: Attempted to acces an index that is greater than size
 * @C_CAP_GREATER: Attempted to expand to a smaller capacity than current
 * @C_CAP_LOWER: Attempted to shrink to a larger capacity than current
 * @C_NOT_IMPLEMENTED: The function itself or a part of it has not implemented
 *                     yet
 */
enum cresult {
	C_OK,
	C_NOMEM,
	C_EMPTY,
	C_OUT_OF_BOUNDS,
	C_CAP_GREATER,
	C_CAP_LOWER,
	C_NOT_IMPLEMENTED,
};

#endif
