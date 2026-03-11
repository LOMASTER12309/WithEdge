#ifndef CORE_CORE_FUNCTIONS_HPP
#define CORE_CORE_FUNCTIONS_HPP

#include <lua5.3/lua.hpp>

namespace core {

/**
 * \brief Запустить операцию saveLayer
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int saveLayer(lua_State* L);

/**
 * \brief Запустить операцию angle
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int angle(lua_State* L);

/**
 * \brief Запустить операцию area
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int area(lua_State* L);

/**
 * \brief Запустить операцию coincidentEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int coincidentEdge(lua_State* L);

/**
 * \brief Запустить операцию coincidentInsideEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int coincidentInsideEdge(lua_State* L);

/**
 * \brief Запустить операцию coincidentOutsideEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int coincidentOutsideEdge(lua_State* L);

/**
 * \brief Запустить операцию cut
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int cut(lua_State* L);

/**
 * \brief Запустить операцию density
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int density(lua_State* L);

/**
 * \brief Запустить операцию donut
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int donut(lua_State* L);

/**
 * \brief Запустить операцию enclose
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int enclose(lua_State* L);

/**
 * \brief Запустить операцию enclosureEdges
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int enclosureEdges(lua_State* L);

/**
 * \brief Запустить операцию enclosureRegions
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int enclosureRegions(lua_State* L);

/**
 * \brief Запустить операцию expandEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int expandEdge(lua_State* L);

/**
 * \brief Запустить операцию extent
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int extent(lua_State* L);

/**
 * \brief Запустить операцию extents
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int extents(lua_State* L);

/**
 * \brief Запустить операцию externalEdges
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int externalEdges(lua_State* L);

/**
 * \brief Запустить операцию externalRegions
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int externalRegions(lua_State* L);

/**
 * \brief Запустить операцию holes
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int holes(lua_State* L);

/**
 * \brief Запустить операцию inside
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int inside(lua_State* L);

/**
 * \brief Запустить операцию insideEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int insideEdge(lua_State* L);

/**
 * \brief Запустить операцию interact
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int interact(lua_State* L);

/**
 * \brief Запустить операцию internalEdges
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int internalEdges(lua_State* L);

/**
 * \brief Запустить операцию internalRegions
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int internalRegions(lua_State* L);

/**
 * \brief Запустить операцию intersection
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int intersection(lua_State* L);

/**
 * \brief Запустить операцию length
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int length(lua_State* L);

/**
 * \brief Запустить операцию notCoincidentEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notCoincidentEdge(lua_State* L);

/**
 * \brief Запустить операцию notCut
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notCut(lua_State* L);

/**
 * \brief Запустить операцию notDonut
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notDonut(lua_State* L);

/**
 * \brief Запустить операцию notInteract
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notInteract(lua_State* L);

/**
 * \brief Запустить операцию notLength
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notLength(lua_State* L);

/**
 * \brief Запустить операцию notOutside
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notOutside(lua_State* L);

/**
 * \brief Запустить операцию notRectangle
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notRectangle(lua_State* L);

/**
 * \brief Запустить операцию notTouch
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notTouch(lua_State* L);

/**
 * \brief Запустить операцию notTouchEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int notTouchEdge(lua_State* L);

/**
 * \brief Запустить операцию offGrid
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int offGrid(lua_State* L);

/**
 * \brief Запустить операцию outside
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int outside(lua_State* L);

/**
 * \brief Запустить операцию outsideEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int outsideEdge(lua_State* L);

/**
 * \brief Запустить операцию pathLength
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int pathLength(lua_State* L);

/**
 * \brief Запустить операцию perimeter
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int perimeter(lua_State* L);

/**
 * \brief Запустить операцию rectangle
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int rectangle(lua_State* L);

/**
 * \brief Запустить операцию size
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int size(lua_State* L);

/**
 * \brief Запустить операцию subtration
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int subtraction(lua_State* L);

/**
 * \brief Запустить операцию touch
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int touch(lua_State* L);

/**
 * \brief Запустить операцию touchEdge
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int touchEdge(lua_State* L);

/**
 * \brief Запустить операцию union
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int _union(lua_State* L);

/**
 * \brief Запустить операцию XOR
 * \param [in] L Указатель на lua_State
 * \return Количество возвращаемых переменных
 */
int XOR(lua_State* L);

} // namespace core

#endif // CORE_CORE_FUNCTIONS_HPP