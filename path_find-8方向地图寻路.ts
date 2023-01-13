


import { BaseColl, BaseKey } from './../base/config';
import { BaseDBPool } from './../base/mongo';
import { BaseRedis } from './../base/redis';
import { GameMap } from './type';


/**
 * 数字键盘：上北下南，左西右东 布局
 * 数组索引，每一个下标表示一个方向数据 (使用数字键盘的数值来索引方向)
 */
const DIR_NULL = -1;        // 表示没有方向数据
const DIR_N = 8;            // 北
const DIR_S = 2;            // 南      
const DIR_W = 4;            // 西
const DIR_E = 6;            // 东
const DIR_SE = 3;           // 东南
const DIR_SW = 1;           // 西南
const DIR_NE = 9;           // 东北
const DIR_NW = 7;           // 西北
// const DIR_CENT = 5;         // 中心位置，5 暂时不用


const DIR_LIST = [DIR_N, DIR_NE, DIR_E, DIR_SE, DIR_S, DIR_SW, DIR_W, DIR_NW];   // 顺时针方向列表查找

const INDEX_NULL = -1;      // 数组索引，表示没有数据
const MAX_HEIGHT = 300;     // 寻路最大高度(防止无限寻找下去)




/**
 * 寻路节点数据
 */
type FindItem = {
    _id: string;
    name: string;
    dir: number[];              // 方向数组，数组索引坐标对应一个方向数据，数组内容对应下个节点在FindMapitems.items中的索引位置 -1 表示没有方向数据

    skipMap?: string;           // 跳转地图名称
    skipItem?: string;          // 跳转的地点名称（跳转点名称不能重复）

    height?: number;            // 寻路节点地势高度(用来判定最优路径)
}

/**
 * 查找地图
 */
type FindMap = {
    _id: string;                // 地图 id
    name: string;               // 地图名称
    skips: FindItem[];          // 跳转出口，当前地图可以跳转到其他地图出口（跳转节点列表）
    items: FindItem[];          // 地图节点数据
}


/**
 * 游戏寻路算法
 */
export class PathFind {

    /**
     * 初始化所有的地图数据到 redis 缓存中
     */
    public static async init() {
        const db = BaseDBPool.getDB();
        const col = db.collection<GameMap>(BaseColl.sysMap);
        const cursor = col.find();
        const redis = BaseRedis.getClient();

        let next = await cursor.next();
        while (next) {
            try {
                const map = next;
                const findMap = this.initMap(map);
                await redis.hSet(BaseKey.pathFind, map.name, JSON.stringify(findMap));
                next = await cursor.next();

            } catch (error) {
                console.error('PathFind.init', error);
            }
        }
    }


    /**
    * 初始化一个地图数据
    * @param map 地图数据
    */
    public static initMap(map: GameMap) {

        // 寻路地图
        const findMap: FindMap = { _id: map._id, name: map.name, items: [], skips: [] };

        for (let index = 0; index < map.items.length; index++) {
            const mapItem = map.items[index];
            const findItem = this.initItem(mapItem._id, mapItem.name);
            findMap.items.push(findItem);
        }

        // 初始化方向数据
        for (let index = 0; index < map.items.length; index++) {
            const mapItem = map.items[index];
            const findItem = findMap.items[index];

            findItem.dir[DIR_N] = this.getItemIndexById(findMap, mapItem.n);
            findItem.dir[DIR_S] = this.getItemIndexById(findMap, mapItem.s);
            findItem.dir[DIR_W] = this.getItemIndexById(findMap, mapItem.w);
            findItem.dir[DIR_E] = this.getItemIndexById(findMap, mapItem.e);
            findItem.dir[DIR_NW] = this.getItemIndexById(findMap, mapItem.nw);
            findItem.dir[DIR_NE] = this.getItemIndexById(findMap, mapItem.ne);
            findItem.dir[DIR_SW] = this.getItemIndexById(findMap, mapItem.sw);
            findItem.dir[DIR_SE] = this.getItemIndexById(findMap, mapItem.se);

            findItem.skipMap = mapItem.skipMap;
            findItem.skipItem = mapItem.skipItem;
        }

        // 初始化跳转数据
        findMap.skips = this.initMapSkip(findMap);

        return findMap;
    }


    /**
     * 当前地图的跳转列表
     * @param map 地图数据
     * @returns 
     */
    public static initMapSkip(map: FindMap) {
        const retSkip = [] as FindItem[];
        if (map) {
            for (let index = 0; index < map.items.length; index++) {
                const item = map.items[index];
                if (item.skipMap) {
                    retSkip.push(item);
                }
            }
        }

        return retSkip;
    }


    // 初始化一个查找单元数据
    public static initItem(id: string, name: string) {
        const findItem: FindItem = {
            _id: id,
            name: name,
            dir: [DIR_NULL, DIR_NULL, DIR_NULL, DIR_NULL, DIR_NULL, DIR_NULL, DIR_NULL, DIR_NULL, DIR_NULL], // 初始化 9个空的数据
        }
        return findItem;
    }

    // 获取单元数据对应的数组下标
    public static getItemIndexById(map: FindMap, id: string) {
        let retIndex = INDEX_NULL;
        if (map && id) {
            for (let index = 0; index < map.items.length; index++) {
                const element = map.items[index];
                if (element._id == id) {
                    retIndex = index;
                    break;
                }
            }
        }
        return retIndex;
    }


    public static getItem(map: FindMap, itemName: string) {
        if (map && itemName) {
            for (let index = 0; index < map.items.length; index++) {
                const item = map.items[index];
                if (item.name == itemName) {
                    return item;
                }
            }
        }
        return null;
    }

    public static getItemBySkip(map: FindMap, skipMap: string) {
        if (map && skipMap) {
            for (let index = 0; index < map.skips.length; index++) {
                const item = map.skips[index];
                if (item.skipMap == skipMap) {
                    return item;
                }
            }
        }
        return null;
    }

    // 获取方向信息
    public static getDir(map: FindMap, from: FindItem, to: FindItem) {
        if (map && from && to) {

            if (from.dir[DIR_N] != DIR_NULL && map.items[from.dir[DIR_N]]._id == to._id) {
                return DIR_N;
            }
            if (from.dir[DIR_S] != DIR_NULL && map.items[from.dir[DIR_S]]._id == to._id) {
                return DIR_S;
            }
            if (from.dir[DIR_W] != DIR_NULL && map.items[from.dir[DIR_W]]._id == to._id) {
                return DIR_W;
            }
            if (from.dir[DIR_E] != DIR_NULL && map.items[from.dir[DIR_E]]._id == to._id) {
                return DIR_E;
            }

            if (from.dir[DIR_NE] != DIR_NULL && map.items[from.dir[DIR_NE]]._id == to._id) {
                return DIR_NE;
            }
            if (from.dir[DIR_NW] != DIR_NULL && map.items[from.dir[DIR_NW]]._id == to._id) {
                return DIR_NW;
            }
            if (from.dir[DIR_SE] != DIR_NULL && map.items[from.dir[DIR_SE]]._id == to._id) {
                return DIR_SE;
            }
            if (from.dir[DIR_SW] != DIR_NULL && map.items[from.dir[DIR_SW]]._id == to._id) {
                return DIR_SW;
            }
        }
        return DIR_NULL;
    }

    /**
     * 获取路径字符串数据
     * @param name 
     */
    public static getDirPath(map: FindMap, items: FindItem[]) {

        let retPath = '';

        // 注意： 查找结果是顺序是倒序的
        for (let index = items.length - 1; index > 0; index--) {
            const item1 = items[index];
            const item2 = items[index - 1];
            const dir = this.getDir(map, item1, item2);
            retPath += dir;
        }

        return retPath;
    }

    /**
     * 获取寻路地图数据
     */
    public static async getMap(name: string) {

        let findMap: FindMap = null;
        if (name) {
            try {
                const redis = BaseRedis.getClient();
                const strValue = await redis.hGet(BaseKey.pathFind, name);
                if (strValue) {
                    findMap = JSON.parse(strValue);
                }
            } catch (error) {
                console.error('PathFind.getMap', error);
            }
        }

        return findMap;
    }


    /**
     * 地图寻路 ，查找从当前地图到目标地图需要经过的地图列表
     * @param from  当前地图名称 
     * @param dist  目标地图名称
     * @param result 查找成功，存储查找的结果 (输出参数)
     * @param check 存储已经查找过的数组数据，防止重复查找
     * @returns true 查找成功， false 查找失败
     * 
     * 
     * 使用举例：
     *      const mapCheck = new Map<string, string>();
            const result = [];
            const retFind = await PathFind.findMapPath('武馆', '少林寺', result, mapCheck)
            console.log({ retFind, result, mapCheck });
     */
    public static async findMapPath(from: string, dist: string, result: FindMap[] = [], check: Map<string, string> = new Map()) {

        // console.log('findMapPath-->', from);

        // 检测是否已经查找过
        if (check.has(from)) {
            return false;
        }

        const map = await this.getMap(from);
        if (map) {
            check.set(from, from);

            // 找到目标
            if (map.name == dist) {
                result.push(map);
                return true;
            }

            for (let index = 0; index < map.skips.length; index++) {
                const skipMap = map.skips[index].skipMap;
                const retFind = await this.findMapPath(skipMap, dist, result, check);
                if (retFind) {   // 递归函数找到目标
                    result.push(map);
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * 查找地图节点路径 (随机一个路径-消耗较小)
     * @param map 地图数据
     * @param from 初始地点 (当前地图上)
     * @param dist 目标地点（当前地图上）
     * @param result 查找成功，存储查找的结果 (输出参数)
     * @param check 存储已经查找过的数组数据，防止重复查找
     * @returns  true 查找成功， false 查找失败
     * 
     * 使用举例： 
     *      const map = await PathFind.getMap('扬州城');
            const item1 = PathFind.getItem(map, '北门');
            const item2 = PathFind.getItem(map, '中央广场xxx');
            // console.log({ map, item1, item2 });
            const mapCheck = new Map<string, string>();
            const result = [];
            const retFind = PathFind.findItemPath(map, item1, item2, result, mapCheck);
            console.log({ retFind, result, mapCheck });
     */
    public static findItemPathRand(map: FindMap, from: FindItem, dist: FindItem, result: FindItem[] = [], check: Map<string, string> = new Map()) {

        // console.log('findItemPathRand-->', from.name, dist.name);

        if (dist == null || from == null || map == null) {
            return false;
        }

        // 检测是否已经查找过
        if (check.has(from._id)) {
            return false;
        }
        check.set(from._id, from.name);

        // 找到了。
        if (from._id == dist._id) {
            result.push(from);
            return true;
        }

        // 八个方向分别查找
        for (let index = 0; index < DIR_LIST.length; index++) {
            const dirIndex = from.dir[DIR_LIST[index]];
            if (dirIndex != DIR_NULL) {
                const retFind = this.findItemPathRand(map, map.items[dirIndex], dist, result, check);
                if (retFind) {   // 递归函数找到目标
                    result.push(from);
                    return true;
                }
            }
        }


        return false;
    }


    /**
     * 查找地图节点路径 （获取一个最优路径）
     * @param map 地图数据
     * @param from 初始地点 (当前地图上)
     * @param dist 目标地点（当前地图上）
     * @param result 查找成功，存储查找的结果 (输出参数)
     * @param check 存储已经查找过的数组数据，防止重复查找
     * @param height 查找的高度，没递归一级，高度 +1
     * @returns  true 查找成功， false 查找失败
     * 
     * 使用举例： 
     *      const map = await PathFind.getMap('扬州城');
            const item1 = PathFind.getItem(map, '北门');
            const item2 = PathFind.getItem(map, '中央广场xxx');
            // console.log({ map, item1, item2 });
            const mapCheck = new Map<string, number>();
            const result = [];
            const retFind = PathFind.findItemPath(map, item1, item2, result, mapCheck);
            console.log({ retFind, result, mapCheck });
     */
    public static findItemPathMin(map: FindMap, from: FindItem, dist: FindItem, result: FindItem[] = [], check: Map<string, number> = new Map(), height: number = 0) {

        // console.log('findItemPathMin-->', from.name, dist.name, height);

        if (dist == null || from == null || map == null) {
            return false;
        }

        if (height > MAX_HEIGHT) {
            return;
        }

        // 检测是否已经查找过 (检查高度)
        if (check.has(from._id) && height >= check.get(from._id)) {
            return false;           // 不是最优路径
        }
        check.set(from._id, height);

        // 找到了。
        if (from._id == dist._id) {
            result.splice(0, result.length);   // 清除之前的路径
            result.push(from);
            return true;
        }

        let childIsFind = false;

        // 八个方向分别查找
        for (let index = 0; index < DIR_LIST.length; index++) {
            const dirIndex = from.dir[DIR_LIST[index]];
            if (dirIndex != DIR_NULL) {
                const retFind = this.findItemPathMin(map, map.items[dirIndex], dist, result, check, height + 1);
                if (retFind) {   // 递归函数找到目标
                    childIsFind = true;
                }
            }
        }

        // 在孩子节点中找到了
        if (childIsFind) {
            result.push(from);
            return true;
        }

        return false;
    }


    /**
     * 地图寻路
     * @param fromMap 当前地图名称
     * @param fromItem 位置名称
     * @param distMap 目标地图名称
     * @param distItem 位置名称
     * @param minPath 是否查找最优路径，注意：最优路径性能消耗比较大！！
     * @returns '' 表示目标不可达， 查找成功返回对应的行走路径字符串
     */
    public static async find(fromMap: string, fromItem: string, distMap: string, distItem: string, minPath = false) {

        let retPath = '';

        const mapList: FindMap[] = [];
        const isFindMapSucc = await PathFind.findMapPath(fromMap, distMap, mapList);
        if (!isFindMapSucc) {
            return '';        // 目标不可达到
        }

        const arrItemAll: FindItem[] = [];            // 当前寻路的所有节点数据

        // 根据地图列表，查找每个地图数据
        let currItemName = fromItem;
        for (let index = mapList.length - 1; index >= 0; index--) {
            const map = mapList[index];
            const mapNext = (index == 0 ? null : mapList[index - 1]);  // 下个地图数据
            const item1 = this.getItem(map, currItemName);
            const item2 = mapNext ? this.getItemBySkip(map, mapNext.name) : this.getItem(map, distItem);
            const itemFindResult: FindItem[] = []

            let isFindItemSucc = false;
            if (minPath) {
                isFindItemSucc = await this.findItemPathMin(map, item1, item2, itemFindResult);
            }
            else {
                isFindItemSucc = await this.findItemPathRand(map, item1, item2, itemFindResult);
            }

            if (isFindItemSucc) {
                retPath += this.getDirPath(map, itemFindResult);
                arrItemAll.push(...itemFindResult);
            }
            else {
                return ''; // 目标不可达到
            }

            if (item2 && item2.skipMap) {
                currItemName = item2.skipItem;
            }
        }

        // console.log('路线-->arrItemAll', arrItemAll);
        // for (let index = 0; index < arrItemAll.length; index++) {
        //     const item = arrItemAll[index];
        //     console.log('路线-->', item.name);
        // }
        
        console.log('retPath', retPath);

        return retPath;
    }

}